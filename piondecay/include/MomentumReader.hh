#ifndef MomentumReader_h
#define MomentumReader_h
#include <vector>
#include <string>

struct PionMomentum {
    double px, py, pz;
    double p_mag;
    double KE;
    double E;
    int    delta_pdg;
};

class MomentumReader {
public:
    MomentumReader(const std::string& filename);
    bool        IsValid() const { return fValid; }
    std::size_t Size()    const { return fEntries.size(); }
    const PionMomentum& GetEntry(std::size_t index) const;
    int GetDeltaPDG(std::size_t index) const {
        return fEntries[index % fEntries.size()].delta_pdg;
    }
private:
    std::vector<PionMomentum> fEntries;
    bool fValid = false;
};
#endif
