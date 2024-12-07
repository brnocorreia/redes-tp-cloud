#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <cmath>
#include <filesystem>
#include <cstring>

namespace fs = std::filesystem;

// Function to generate a random string of a given length
std::string generateRandomString(size_t length) {
    const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, characters.size() - 1);

    std::string randomString;
    for (size_t i = 0; i < length; ++i) {
        randomString += characters[distribution(generator)];
    }
    return randomString;
}

void createJsonFiles(const std::string& folder, int i_size) {
    // Verify if the folder exists, if not, create it
    if (!fs::exists(folder)) {
        fs::create_directories(folder);
        std::cout << "Folder created: " << folder << "\n";
    }

    for (int j = 0; j < 64; ++j) {
        // Generate a random name for the file
        std::string randomName = generateRandomString(i_size - 6); // Subtraindo 6 por causa de ".json"
        std::string fileName = randomName + ".json";

        // Path to the file
        std::string filePath = folder + "/" + fileName;

        // Create the empty file
        std::ofstream outFile(filePath);
        if (!outFile) {
            std::cerr << "Error while creating file: " << filePath << "\n";
        } else {
            outFile.close();
        }
    }

    std::cout << "Test archives created successfully! Check the -> " << folder << " folder" << "\n";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << "<buffer_size>" << std::endl;
        return 1;
    }

    int i_size = std::stoi(argv[1]);
    int buffer_size = std::pow(2, i_size);
    if (buffer_size >= 256) {
        buffer_size = 255;
    }

    try {
        const std::string folder = "./tests";
        createJsonFiles(folder, buffer_size);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
