#include "MomentumReader.hh"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

MomentumReader::MomentumReader(const std::string& filename)
{
    std::ifstream f(filename);
    if (!f.is_open()) {
        std::cerr << "[MomentumReader] ERROR: cannot open '" << filename << "'\n";
        return;
    }

    std::string line;
    // Skip header line
    std::getline(f, line);

    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string tok;
        PionMomentum pm;

        // CSV columns: px_GeV, py_GeV, pz_GeV, p_mag_GeV, KE_GeV, E_GeV
        try {
            std::getline(ss, tok, ','); pm.px    = std::stod(tok);
            std::getline(ss, tok, ','); pm.py    = std::stod(tok);
            std::getline(ss, tok, ','); pm.pz    = std::stod(tok);
            std::getline(ss, tok, ','); pm.p_mag = std::stod(tok);
            std::getline(ss, tok, ','); pm.KE    = std::stod(tok);
            std::getline(ss, tok, ','); pm.E     = std::stod(tok);
            fEntries.push_back(pm);
        } catch (...) {
            std::cerr << "[MomentumReader] Warning: skipping malformed line\n";
        }
    }

    fValid = !fEntries.empty();
    std::cout << "[MomentumReader] Loaded " << fEntries.size()
              << " entries from '" << filename << "'\n";
}

const PionMomentum& MomentumReader::GetEntry(std::size_t index) const
{
    return fEntries[index % fEntries.size()];
}
