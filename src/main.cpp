/**
 *  Dependencies
 */
#include "includes/CImage.h"
#include <iostream>
#include <string>

#include <curl/curl.h>
#include <curl/multi.h>

/*
 * Namespace convenience
 */
using namespace std;
using namespace CImageNamespace;

/**
 *  Main program
 *  @return int
 */
int main(int argc, char **argv) {
    if(argc != 2 && argc != 3){
        cout << "Invalid args, e.g,: CImage <input image path> [<output image path>]" << endl;
        return 1;
    }

    // Download/read file, compress it dynamically and store at path ./output.jpg
    // Download/read file, compress it dynamically and store at given path

    // Curl init
    curl_global_init(CURL_GLOBAL_ALL);

    string output_path = "./output.jpg";
    if(argc == 3){
        output_path = argv[2];
    }

    CImage img(argv[1], output_path);

    // float simlarity_threshold = IMAGE_SIMILARITY_THRESHOLD,int low_quality = MIN_IMAGE_QUALITY, int high_quality = MAX_IMAGE_QUALITY, int quality_step = IMAGE_QUALITY_STEP
    img.dynamically_compress_image();

    // Close curl
    curl_global_cleanup();

    return 0;
}
