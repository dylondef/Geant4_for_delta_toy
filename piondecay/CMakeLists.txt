cmake_minimum_required(VERSION 3.16)
project(PionDecay)

# ── Geant4 ────────────────────────────────────────────────────────────────────
find_package(Geant4 REQUIRED)
include(${Geant4_USE_FILE})

# ── ROOT (optional but required for .root output) ─────────────────────────────
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree)
include(${ROOT_USE_FILE})

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCES
    src/DetectorConstruction.cc
    src/PrimaryGeneratorAction.cc
    src/RunAction.cc
    src/EventAction.cc
    src/TrackingAction.cc
    src/SteppingAction.cc
    src/ActionInitialization.cc
    src/CherenkovAnalyzer.cc
    src/MomentumReader.cc
    src/CoPrimaryReader.cc
)

add_executable(piondecay piondecay.cc ${SOURCES})
target_link_libraries(piondecay ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})

install(TARGETS piondecay DESTINATION bin)
