cmake_minimum_required(VERSION 4.3.2)

project(soloud)

set(SOLOUD_SOURCES
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_wav.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_waveshaperfilter.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_audiosource.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_flangerfilter.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_fftfilter.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_wavstream.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_eqfilter.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_sfxr.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_echofilter.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_monotone.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_duckfilter.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_queue.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_noise.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_biquadresonantfilter.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_lofifilter.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_fader.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_fft.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_vizsn.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_freeverbfilter.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_error.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_bus.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_vic.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_c.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_ay.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_file_hack_off.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_tedsid.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_bassboostfilter.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_dcremovalfilter.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_misc.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_internal.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_filter.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_file.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_file_hack_on.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/zx7decompress.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_robotizefilter.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_thread.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_speech.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include/soloud_openmpt.h"

    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_basicops.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_queue.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_fft_lut.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_filter.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_setters.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_misc.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_faderops.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_getters.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_fft.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_audiosource.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_fader.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_file.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_filterops.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_3d.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_bus.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_voiceops.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_thread.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/core/soloud_core_voicegroup.cpp"
    
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/soloud_wavstream.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/dr_wav.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/dr_impl.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/stb_vorbis.c"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/soloud_wav.cpp"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/dr_flac.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/dr_mp3.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/audiosource/wav/stb_vorbis.h"

    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/audiosource/openmpt/soloud_openmpt_dll.c"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/audiosource/openmpt/soloud_openmpt.cpp"

    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/backend/miniaudio/miniaudio.h"
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/src/backend/miniaudio/soloud_miniaudio.cpp"
)

add_library(${PROJECT_NAME} STATIC ${SOLOUD_SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    # All configurations
    WITH_MINIAUDIO
)

target_compile_options(${PROJECT_NAME} PRIVATE
    # ALL configurations
    # -Wall -Wextra
    
    # Debug
    $<$<CONFIG:debug>:-DDEBUG -O0 -g>

    # Release
    $<$<CONFIG:release>:-DNDEBUG -O2>
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${MAG_ROOT_DIR}/libs/${PROJECT_NAME}/include"
)
