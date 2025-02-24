#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <zlib.h>

using namespace std;

const int CHUNK_SIZE = 16384;

void compressChunk(const vector<char>& data, vector<char>& compressedData) {
    uLongf compressedSize = compressBound(data.size());
    compressedData.resize(compressedSize);
    
    if (compress(reinterpret_cast<Bytef*>(compressedData.data()), &compressedSize,
                 reinterpret_cast<const Bytef*>(data.data()), data.size()) != Z_OK) {
        cerr << "Compression failed!" << endl;
    }
    compressedData.resize(compressedSize);
}

void decompressChunk(const vector<char>& compressedData, vector<char>& decompressedData, uLong originalSize) {
    decompressedData.resize(originalSize);
    
    if (uncompress(reinterpret_cast<Bytef*>(decompressedData.data()), &originalSize,
                    reinterpret_cast<const Bytef*>(compressedData.data()), compressedData.size()) != Z_OK) {
        cerr << "Decompression failed!" << endl;
    }
    decompressedData.resize(originalSize);
}

void compressFile(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile, ios::binary);
    ofstream out(outputFile, ios::binary);
    
    if (!in || !out) {
        cerr << "Error opening files!" << endl;
        return;
    }
    
    vector<thread> threads;
    vector<vector<char>> compressedChunks;
    
    while (!in.eof()) {
        vector<char> chunk(CHUNK_SIZE);
        in.read(chunk.data(), CHUNK_SIZE);
        chunk.resize(in.gcount());
        
        compressedChunks.emplace_back();
        threads.emplace_back(compressChunk, ref(chunk), ref(compressedChunks.back()));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    for (const auto& compressedChunk : compressedChunks) {
        out.write(compressedChunk.data(), compressedChunk.size());
    }
    
    cout << "File compressed successfully!" << endl;
}

void decompressFile(const string& inputFile, const string& outputFile, uLong originalSize) {
    ifstream in(inputFile, ios::binary);
    ofstream out(outputFile, ios::binary);
    
    if (!in || !out) {
        cerr << "Error opening files!" << endl;
        return;
    }
    
    vector<thread> threads;
    vector<vector<char>> decompressedChunks;
    
    while (!in.eof()) {
        vector<char> chunk(CHUNK_SIZE);
        in.read(chunk.data(), CHUNK_SIZE);
        chunk.resize(in.gcount());
        
        decompressedChunks.emplace_back();
        threads.emplace_back(decompressChunk, ref(chunk), ref(decompressedChunks.back()), originalSize);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    for (const auto& decompressedChunk : decompressedChunks) {
        out.write(decompressedChunk.data(), decompressedChunk.size());
    }
    
    cout << "File decompressed successfully!" << endl;
}

int main() {
    string inputFile = "input.txt";
    string compressedFile = "compressed.dat";
    string decompressedFile = "decompressed.txt";
    
    cout << "Compressing file..." << endl;
    compressFile(inputFile, compressedFile);
    
    cout << "Decompressing file..." << endl;
    decompressFile(compressedFile, decompressedFile, CHUNK_SIZE);
    
    return 0;
}