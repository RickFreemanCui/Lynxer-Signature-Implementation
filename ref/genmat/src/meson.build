# path string for include folder
header_folder_path = meson.project_source_root() / 'include'
# all header files for this library
lib_header_files = files(
    header_folder_path / 'Common.hpp',
    header_folder_path / 'NtlCommon.h',
)
#boost_root_path=getenv('BOOST_ROOT')
boost_deps =  dependency('boost', 
  modules : ['serialization', 'filesystem', 'program_options', 'iostreams']
)
# thread_deps = cxx.find_library('pthread', required: true)
cms_deps = cxx.find_library('cryptominisat5', dirs: '/usr/local/lib')
ntl_deps = cxx.find_library('ntl', dirs: '/usr/local/lib')
thread_deps = cxx.find_library('pthread', required: true)
gmp_deps = cxx.find_library('gmp', required: true)
# all source for this library include header files
lib_source_files = files(
    'Common.cpp',
    'NtlCommon.cpp',
    ) + lib_header_files
# config library
common_lib = static_library(
    'CommonUtils',
    lib_source_files,
    include_directories: include_inc,
    dependencies: [
        boost_deps,
        cms_deps,
        ntl_deps,
        thread_deps,
        gmp_deps,
        ],   
    install: true,
)