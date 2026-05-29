boost_test = dependency('boost', required: false, modules: ['unit_test_framework'], version: '>=1.81')
ntl = cc.find_library('ntl', required: false, has_headers: ['NTL/version.h'])

if boost_test.found()
  test_sources = files(
    'aes.cpp',
    # 'aes_prove.cpp',
    'bavc.cpp',
    'extended_tests.cpp',
    'fields.cpp',
    'hash_shake.cpp',
    'instances.cpp',
    'owf.cpp',
    'prg.cpp',
    'universal_hashing.cpp',
    'utils.cpp',
    'vole.cpp',
  )
  test_deps = []
  test_defines = ['-DBOOST_TEST_DYN_LINK', '-DFAEST_TESTS']
  if ntl.found()
    test_deps += ntl
    test_defines += ['-DHAVE_NTL']
  endif

  foreach parameter_set : parameter_sets
    if parameter_set.get('PARAM_L') == 'lynx_512s'
      continue
    endif
    test_sources += configure_file(
      input: 'basic.cpp.in',
      output: 'faest_@0@_basic.cpp'.format(parameter_set.get('PARAM_L')),
      configuration: parameter_set,
    )
  endforeach

  extended_tests = executable(
    'test_extended_tests',
    test_sources,
    dependencies: [libfaest_static_dependency, boost_test] + test_deps,
    include_directories: include_directories,
    cpp_args: defines + cpp_flags + test_defines,
    link_args: linker_flags,
    override_options: ['b_lto=false'],
  )
  test(
    'AES',
    extended_tests,
    args: ['-t', 'aes'],
    timeout: 6000,
  )
  # only useful for debugging
  # test(
  #   'AES prove',
  #   extended_tests,
  #   args: ['-t', 'test_aes_prove'],
  #   timeout: 6000,
  # )
  test(
    'SHAKE',
    extended_tests,
    args: ['-t', 'hash_shake'],
    timeout: 6000,
  )
  test(
    'Instances',
    extended_tests,
    args: ['-t', 'instances'],
    timeout: 6000,
  )
  # OWF test vectors intentionally disabled: the OWF definition changes frequently.
  # Keep tests/owf.cpp in the binary so it can still be run manually when TVs are refreshed.
  test(
    'PRG',
    extended_tests,
    args: ['-t', 'prgs'],
    timeout: 6000,
  )
  test(
    'Universal hashing',
    extended_tests,
    args: ['-t', 'universal_hashing'],
    timeout: 6000,
  )
  test(
    'Utils',
    extended_tests,
    args: ['-t', 'utils'],
    timeout: 6000,
  )
  test(
    'BAVC',
    extended_tests,
    args: ['-t', 'bavc'],
    timeout: 6000,
  )
  test(
    'VOLE',
    extended_tests,
    args: ['-t', 'vole'],
    timeout: 6000,
  )
  if ntl.found()
    test(
      'Fields',
      extended_tests,
      args: ['-t', 'fields'],
      timeout: 6000,
    )
  endif
  foreach parameter_set : parameter_sets
    if not parameter_set.get('PARAM_L').startswith('lynx_')
      test(
        'FAEST_@0@'.format(parameter_set.get('PARAM')),
        extended_tests,
        args: ['-t', 'faest_@0@'.format(parameter_set.get('PARAM_L'))],
        timeout: 6000,
      )
    endif
  endforeach
endif

if get_option('tv-generators').enabled()
  tv_generator_deps = [libfaest_static_dependency]
  if openssl.found()
    tv_generator_deps += openssl
  endif

  executable(
    'generate_bavc_tv',
    files('generate_bavc_tv.cpp'),
    dependencies: tv_generator_deps,
    include_directories: include_directories,
    cpp_args: defines + test_defines + cpp_flags,
    link_args: linker_flags,
    override_options: ['b_lto=false'],
  )
  executable(
    'generate_prg_tv',
    files('generate_prg_tv.cpp'),
    dependencies: tv_generator_deps,
    include_directories: include_directories,
    cpp_args: defines + test_defines + cpp_flags,
    link_args: linker_flags,
    override_options: ['b_lto=false'],
  )
  executable(
    'generate_owf_tv',
    files('generate_owf_tv.cpp'),
    dependencies: tv_generator_deps,
    include_directories: include_directories,
    cpp_args: defines + test_defines + cpp_flags,
    link_args: linker_flags,
    override_options: ['b_lto=false'],
  )
  executable(
    'generate_vole_tv',
    files('generate_vole_tv.cpp'),
    dependencies: tv_generator_deps,
    include_directories: include_directories,
    cpp_args: defines + test_defines + cpp_flags,
    link_args: linker_flags,
    override_options: ['b_lto=false'],
  )
  executable(
    'generate_universal_hashing_tv',
    files('generate_universal_hashing_tv.cpp'),
    dependencies: tv_generator_deps,
    include_directories: include_directories,
    cpp_args: defines + test_defines + cpp_flags,
    link_args: linker_flags,
    override_options: ['b_lto=false'],
  )
endif
