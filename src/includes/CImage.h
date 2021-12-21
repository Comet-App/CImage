
#ifndef C_IMAGE_H
#define C_IMAGE_H

#include "Magick++/Image.h"
#include "Magick++/Include.h"
#include "MagickCore/compare.h"
#include <Magick++.h>
#include <exception>
#include <stdexcept>
#include <string>
#include <sys/stat.h>

namespace CImageNamespace {
enum COMPARISON_METRICS { SSIM, PSNR, MAE };

const unsigned int MIN_IMAGE_QUALITY = 30;
const unsigned int MAX_IMAGE_QUALITY = 100;
const unsigned int IMAGE_QUALITY_STEP = 10;
const float IMAGE_SIMILARITY_THRESHOLD = 0.9665;


class CImage {
  private:
    std::string path;
    std::string uri;
    std::string output_path;
    Magick::Image image;

    void load_image_from_path();

    bool write_to_file(std::string new_path = "");

    Magick::Image compress_image(Magick::Image img, int quality);

    double compare_image(Magick::Image img2, COMPARISON_METRICS metric = SSIM);

    bool download_image_from_url();

    // Inline functions
    inline bool file_path_exists(const char *file_path) {
        struct stat buffer;
        return stat(file_path, &buffer) == 0;
    }

  public:
    CImage(std::string url_or_path, std::string output_path = "");
    CImage(Magick::Blob blob, std::string output_path = "");
    CImage(Magick::Image img, std::string output_path);
    int dynamically_compress_image(float simlarity_threshold = IMAGE_SIMILARITY_THRESHOLD,int low_quality = MIN_IMAGE_QUALITY, int high_quality = MAX_IMAGE_QUALITY, int quality_step = IMAGE_QUALITY_STEP);

    // Inline functions
    inline ~CImage() { this->write_to_file(); }
    inline bool save(std::string new_path = "") {
        return this->write_to_file(new_path);
    }
    inline std::string get_output_path() { return this->output_path; }
    inline Magick::Blob get_image_blob() {
        Magick::Blob blob;
        this->image.write(&blob);

        return blob;
    }
    inline void set_image_quality(int quality = -1) {
        if (quality > 100 || quality < 0) {
            return;
        }
        Magick::Image new_image = this->get_image(quality);
        this->image = new_image;
    }
    inline Magick::Image get_image(int quality = -1) {
        if (quality > 100 || quality < 10) {
            return this->image;
        }

        return this->compress_image(this->image, quality);
    }
};
} // namespace CImageNamespace

#endif