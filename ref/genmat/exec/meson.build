gmat_source_files=files(
    'genMat/mainMatTest.cpp',
    'genMat/LinPolyGF2E.cpp',
    'genMat/LinPolyGF2E.h',
    )

executable(
    'genMat',
    gmat_source_files,
    include_directories: [
        include_inc,
    ],
    dependencies: [boost_deps],
    link_language: 'cpp',
    link_with: [common_lib],
    install: false,
)

