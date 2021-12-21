#include "./includes/CImage.h"
#include "Magick++/Blob.h"
#include <cstddef>
#include <curl/curl.h>
#include <curl/multi.h>
#include <iostream>
#include <fstream>
#include <istream>
#include <streambuf>
#include <thread>

using namespace CImageNamespace;
using namespace std;

unsigned int compare_doubles(double a, double b, int to_places = 3) {
    const double tens_factor = pow(10.0, to_places);
    const int a_int = static_cast<int>(a * tens_factor);
    const int b_int = static_cast<int>(b * tens_factor);

    if (a_int == b_int)
        return 0;
    if (a_int > b_int) {
        return -1;
    }
    return 1;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    if (ptr) {
        ((std::string *)stream)->append((char *)ptr, size * nmemb);
        return (size * nmemb);
    }
    return 0;
}

void CImage::load_image_from_path() {
    Magick::Image img;
    try {
        img.read(this->path);
        this->image = img;
    } catch (std::exception e) {
        cout << "Failed to load image cause: " << e.what() << endl;
        throw std::runtime_error("Failed to load image");
    }
}

bool CImage::write_to_file(std::string new_path) {
    if (new_path != "" && new_path != this->output_path) {
        this->output_path = new_path;
    }

    if (this->output_path != "") {
        this->image.write(this->output_path);
        return true;
    }
    return false;
}

Magick::Image CImage::compress_image(Magick::Image img, int quality) {
    // Cloning this terminal command: magick source.jpg -strip -interlace
    // Plane -gaussian-blur 0.05 -quality 85% result.jpg into API mode
    // Stackoverflow answer:
    // https://stackoverflow.com/questions/7261855/recommendation-for-compressing-jpg-files-with-imagemagick

    // Image compression begins

    img.magick("JPEG");                           // Set image format to jpeg
    img.strip();                                  // Strips exif metadata
    img.quality(quality);                         // Setting image quality
    img.samplingFactor("4:2:0");                  // Sets the sampling factor
    img.defineSet("jpeg", "dct-method", "float"); // Setting define set
    img.interlaceType(Magick::PlaneInterlace); // Setting interlace | this also
                                               // sets image type to progressive

    Magick::Blob temp_blob;
    img.write(&temp_blob);

    Magick::Image compressed_img;
    compressed_img.read(temp_blob);

    return compressed_img;
}

double CImage::compare_image(Magick::Image img2, COMPARISON_METRICS metric) {
    switch (metric) {
    case SSIM:
        return this->image.compare(img2,
                                   MagickCore::StructuralSimilarityErrorMetric);
    case PSNR:
        return this->image.compare(
            img2, MagickCore::PeakSignalToNoiseRatioErrorMetric);
    case MAE:
        return this->image.compare(img2, MagickCore::MeanAbsoluteErrorMetric);
    }
    return 0.0;
}

bool CImage::download_image_from_url() {
    if (this->uri == "") {
        return false;
    }

    cout << "Downloading image from " << this->uri << endl;

    std::string readBuffer;
    CURLcode res;
    CURL *curl_handle = curl_easy_init();

    if (curl_handle) {
        // curl_multi_add_handle()
        curl_easy_setopt(curl_handle, CURLOPT_URL, this->uri.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, true);
        // Use this method to do progress call back if required
        // curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION,
        // progress_func);

        res = curl_easy_perform(curl_handle);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        if (res != CURLE_OK) {
            curl_easy_cleanup(curl_handle);
            cout << "Error downloading image from url " << this->uri << " | Status code: " << res << endl;
            throw std::runtime_error("Error downloading image from url");
        } else {
            long response_code;
            curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE,
                              &response_code);
            curl_easy_cleanup(curl_handle);

            if (response_code != 200) {
                cout << "Got response code : " << response_code << ". Failed to download image from url " << this->uri << endl;
                throw std::runtime_error("Error downloading image from url");
            } else {
                const size_t data_length = readBuffer.length();
                Magick::Blob image_blob(readBuffer.c_str(), data_length);
                this->image.read(image_blob);
            }
        }
    }
    return true;
}

CImage::CImage(std::string url_or_path, std::string output_path) {
    if (url_or_path.rfind("http://") == 0 ||
        url_or_path.rfind("https://") == 0) {
        // It's a url
        this->uri = url_or_path;

        // Check if file exists
        if (!this->download_image_from_url()) {
            throw std::runtime_error("Failed to download image from url");
        }
    } else {
        if (this->file_path_exists(url_or_path.c_str()) == false) {
            throw std::runtime_error("Invalid file path, can't read");
        }
        // It's a path
        this->path = url_or_path;

        // Try loading image
        this->load_image_from_path();
    }

    this->output_path = output_path;
}

CImage::CImage(Magick::Blob blob, std::string output_path) {
    Magick::Image img;
    try {
        img.read(blob);
        this->image = img;
    } catch (std::exception e) {
        cout << "Failed to load image from passed blob object. Error: " << e.what() << endl;
        throw std::runtime_error(
            "Failed to load image from passed blob object");
    }

    this->output_path = output_path;
}

CImage::CImage(Magick::Image img, std::string output_path) {
    this->image = img;
    this->output_path = output_path;
}

int CImage::dynamically_compress_image(float simlarity_threshold, int low_quality, int high_quality, int quality_step) {

    unsigned int lo = low_quality, hi = high_quality;
    unsigned int prev_quality = -1;
    unsigned int mid = prev_quality;

    while (lo <= hi) {
        mid = (lo + hi) / 2;

        Magick::Image compressed_image = this->compress_image(this->image, mid);
        const double comparision_score = this->compare_image(compressed_image);

        const int double_diff = compare_doubles(
            comparision_score, simlarity_threshold, 5);

        if (double_diff == 0) {
            prev_quality = mid;
            break;
        } else if (double_diff == -1) {
            // Search in the right side of arary
            prev_quality = mid;
            hi = mid - quality_step;
        } else {
            // Search in the left side of array
            lo = mid + quality_step;
        }
    }

    if (prev_quality != -1 && prev_quality > 0) {
        this->set_image_quality(prev_quality);
    } else {
        cout << "No new image quality found for score " << simlarity_threshold << endl;
    }

    return prev_quality;
}
