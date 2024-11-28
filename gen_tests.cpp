#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <cmath>
#include <filesystem>

namespace fs = std::filesystem;

// Função para gerar uma string aleatória com tamanho especificado
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

void createJsonFiles(const std::string& folder, int i) {
    // Verifica e cria a pasta, se necessário
    if (!fs::exists(folder)) {
        fs::create_directories(folder);
        std::cout << "Pasta criada: " << folder << "\n";
    }

    // Tamanho do nome do arquivo, incluindo extensão ".json"
    size_t nameLength = std::pow(2, i);

    for (int j = 0; j < 50; ++j) {
        // Gera um nome aleatório com tamanho suficiente para totalizar `nameLength` bytes
        std::string randomName = generateRandomString(nameLength - 5); // Subtraindo 5 por causa de ".json"
        std::string fileName = randomName + ".json";

        // Caminho completo do arquivo
        std::string filePath = folder + "/" + fileName;

        // Cria o arquivo vazio
        std::ofstream outFile(filePath);
        if (!outFile) {
            std::cerr << "Erro ao criar o arquivo: " << filePath << "\n";
        } else {
            outFile.close();
        }
    }

    std::cout << "50 arquivos criados na pasta " << folder << "\n";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <valor_de_i>\n";
        return 1;
    }

    try {
        int i = std::stoi(argv[1]);
        const std::string folder = "./tests";
        createJsonFiles(folder, i);
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
