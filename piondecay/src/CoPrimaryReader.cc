#include "CoPrimaryReader.hh"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

CoPrimaryReader::CoPrimaryReader(const std::string& filename)
{
    std::ifstream f(filename);
    if (!f.is_open()) {
        std::cerr << "[CoPrimaryReader] ERROR: cannot open '" << filename << "'\n";
        return;
    }

    // ── Parse header → column index map ──────────────────────────────────────
    std::string header;
    std::getline(f, header);

    std::vector<std::string> cols;
    {
        std::istringstream hss(header);
        std::string tok;
        while (std::getline(hss, tok, ',')) {
            tok.erase(0, tok.find_first_not_of(" \t\r\n"));
            tok.erase(tok.find_last_not_of(" \t\r\n") + 1);
            cols.push_back(tok);
        }
    }

    auto idx = [&](const std::string& name) -> int {
        for (int i = 0; i < (int)cols.size(); ++i)
            if (cols[i] == name) return i;
        return -1;   // column not present — optional fields return -1
    };

    // Required
    int iEnu   = idx("Enu");
    int imu_px = idx("mu_px"), imu_py = idx("mu_py"), imu_pz = idx("mu_pz");
    int ipi_px = idx("pi_px"), ipi_py = idx("pi_py"), ipi_pz = idx("pi_pz");

    if (iEnu<0 || imu_px<0 || imu_py<0 || imu_pz<0 ||
        ipi_px<0 || ipi_py<0 || ipi_pz<0) {
        std::cerr << "[CoPrimaryReader] ERROR: missing required columns\n"
                  << "  Need: Enu, mu_px/py/pz, pi_px/py/pz\n";
        return;
    }

    // Optional — gracefully absent if CSV was built without them
    int iwgt   = idx("wgt");
    int imu_E  = idx("mu_E"),  imu_p  = idx("mu_p");
    int ipi_E  = idx("pi_E"),  ipi_p  = idx("pi_p");
    int icos   = idx("cos_open");
    int iang   = idx("open_angle_deg");
    int ivtx_x = idx("vtx_x"), ivtx_y = idx("vtx_y"), ivtx_z = idx("vtx_z");

    bool hasVtx = (ivtx_x >= 0 && ivtx_y >= 0 && ivtx_z >= 0);
    if (!hasVtx) {
        std::cout << "[CoPrimaryReader] NOTE: vtx_x/y/z not in CSV — "
                  << "firing all events from (0,0,0)\n";
    }

    // ── Parse rows ────────────────────────────────────────────────────────────
    std::string line;
    int linenum = 1;
    while (std::getline(f, line)) {
        ++linenum;
        if (line.empty() || line[0] == '#') continue;

        std::vector<double> v;
        std::istringstream ss(line);
        std::string tok;
        while (std::getline(ss, tok, ',')) {
            try   { v.push_back(std::stod(tok)); }
            catch (...) { v.push_back(0.0); }
        }

        int maxRequired = std::max({iEnu, imu_px, imu_py, imu_pz,
                                    ipi_px, ipi_py, ipi_pz});
        if ((int)v.size() <= maxRequired) {
            std::cerr << "[CoPrimaryReader] Warning: short line "
                      << linenum << " — skipping\n";
            continue;
        }

        auto get = [&](int i) -> double {
            return (i >= 0 && i < (int)v.size()) ? v[i] : 0.0;
        };

        CoPrimaryEvent ev;
        ev.Enu    = get(iEnu);
        ev.wgt    = (iwgt >= 0) ? get(iwgt) : 1.0;

        // Vertex (cm) — defaults to 0,0,0 if not in CSV
        ev.vtx_x  = get(ivtx_x);
        ev.vtx_y  = get(ivtx_y);
        ev.vtx_z  = get(ivtx_z);

        ev.mu_px  = get(imu_px);
        ev.mu_py  = get(imu_py);
        ev.mu_pz  = get(imu_pz);
        ev.mu_p   = (imu_p >= 0) ? get(imu_p) :
                    std::sqrt(ev.mu_px*ev.mu_px + ev.mu_py*ev.mu_py +
                              ev.mu_pz*ev.mu_pz);
        ev.mu_E   = (imu_E >= 0) ? get(imu_E) :
                    std::sqrt(ev.mu_p*ev.mu_p + 0.105658*0.105658);

        ev.pi_px  = get(ipi_px);
        ev.pi_py  = get(ipi_py);
        ev.pi_pz  = get(ipi_pz);
        ev.pi_p   = (ipi_p >= 0) ? get(ipi_p) :
                    std::sqrt(ev.pi_px*ev.pi_px + ev.pi_py*ev.pi_py +
                              ev.pi_pz*ev.pi_pz);
        ev.pi_E   = (ipi_E >= 0) ? get(ipi_E) :
                    std::sqrt(ev.pi_p*ev.pi_p + 0.139570*0.139570);

        ev.cos_open       = (icos >= 0) ? get(icos) : 1.0;
        ev.open_angle_deg = (iang >= 0) ? get(iang) : 0.0;

        fEntries.push_back(ev);
    }

    fValid = !fEntries.empty();
    std::cout << "[CoPrimaryReader] Loaded " << fEntries.size()
              << " events from '" << filename << "'"
              << (hasVtx ? " (with vertex positions)" : " (center vertex)")
              << "\n";
}

const CoPrimaryEvent& CoPrimaryReader::GetEntry(std::size_t index) const
{
    return fEntries[index % fEntries.size()];
}
