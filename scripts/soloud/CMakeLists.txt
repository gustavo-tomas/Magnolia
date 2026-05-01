cmake_minimum_required(VERSION 4.3.2)

project(soloud)

set(SOLOUD_SOURCES
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_wav.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_waveshaperfilter.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_audiosource.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_flangerfilter.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_fftfilter.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_wavstream.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_eqfilter.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_sfxr.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_echofilter.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_monotone.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_duckfilter.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_queue.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_noise.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_biquadresonantfilter.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_lofifilter.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_fader.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_fft.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_vizsn.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_freeverbfilter.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_error.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_bus.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_vic.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_c.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_ay.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_file_hack_off.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_tedsid.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_bassboostfilter.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_dcremovalfilter.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_misc.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_internal.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_filter.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_file.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_file_hack_on.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/zx7decompress.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_robotizefilter.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_thread.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_speech.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include/soloud_openmpt.h"

    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_basicops.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_queue.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_fft_lut.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_filter.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_setters.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_misc.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_faderops.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_getters.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_fft.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_audiosource.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_fader.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_file.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_filterops.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_3d.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_bus.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_voiceops.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_thread.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_voicegroup.cpp"
    
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/soloud_wavstream.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/dr_wav.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/dr_impl.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/stb_vorbis.c"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/soloud_wav.cpp"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/dr_flac.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/dr_mp3.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/stb_vorbis.h"

    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/audiosource/openmpt/soloud_openmpt_dll.c"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/audiosource/openmpt/soloud_openmpt.cpp"

    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/backend/miniaudio/miniaudio.h"
    "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/src/backend/miniaudio/soloud_miniaudio.cpp"
)

add_library(${PROJECT_NAME} STATIC ${SOLOUD_SOURCES})

add_compile_definitions(
    "WITH_MINIAUDIO"
)

target_include_directories(${PROJECT_NAME}
    PRIVATE "${CMAKE_SOURCE_DIR}/libs/${PROJECT_NAME}/include"
)
