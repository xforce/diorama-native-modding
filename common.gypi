{
  'includes': ['toolchain.gypi'],
  'variables': {
    'debug_warning_level_win%': '4',
    'component%': 'static_library',
    'msvs_multi_core_compile%': '1',
    'optimized_debug%': '0',
    'variables': {
      'variables': {
        'variables': {
          'conditions': [
            ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or \
               OS=="netbsd" or OS=="mac" or OS=="qnx"', {
              # This handles the Unix platforms we generally deal with.
              # Anything else gets passed through, which probably won't work
              # very well; such hosts should pass an explicit target_arch
              # to gyp.
              'host_arch%': '<!pymod_do_main(detect_host_arch)',
            }, {
              # OS!="linux" and OS!="freebsd" and OS!="openbsd" and
              # OS!="netbsd" and OS!="mac"
              'host_arch%': 'ia32',
            }],
          ],
        },
        'host_arch%': '<(host_arch)',
        'target_arch%': '<(host_arch)',
      },
      'host_arch%': '<(host_arch)',
      'target_arch%': '<(target_arch)',
    },
    'host_arch%': '<(host_arch)',
    'target_arch%': '<(target_arch)',
    'werror%': '-Werror',
},




'target_defaults': {
  'default_configuration': 'Debug',
  'configurations': {
    'DebugBaseCommon': {
      'cflags': [ '-g', '-O0' ],
      'conditions': [
        ['(target_arch=="ia32" or target_arch=="x87") and \
          OS=="linux"', {
          'defines': [
            '_GLIBCXX_DEBUG'
          ],
        }],
      ],
    },
    'Optdebug': {
      'inherit_from': [ 'DebugBaseCommon', 'DebugBase2' ],
    },
    'Debug': {
      # Xcode insists on this empty entry.
    },
    'Release': {
      # Xcode insists on this empty entry.
    },
  },
  'conditions': [
    ['target_arch=="x64"', {
      'defines': [
        'TARGET_ARCH_X64',
      ],
      'xcode_settings': {
        'ARCHS': [ 'x86_64' ],
      },
      'msvs_settings': {
        'VCLinkerTool': {
          'StackReserveSize': '2097152',
        },
      },
      'msvs_configuration_platform': 'x64',
    }],  # target_arch=="x64"
  ],
},
'conditions': [
  ['OS=="win"', {
    'target_defaults': {
      'defines': [
        '_CRT_SECURE_NO_DEPRECATE',
        '_CRT_NONSTDC_NO_DEPRECATE',
        '_USING_V110_SDK71_',
      ],
      'msvs_configuration_attributes': {
        'CharacterSet': '1',
      },
      'msvs_settings': {
        'VCCLCompilerTool': {
          'MinimalRebuild': 'false',
          'BufferSecurityCheck': 'true',
          'EnableFunctionLevelLinking': 'true',
          'RuntimeTypeInfo': 'true',
          'DebugInformationFormat': '3',
          'Detect64BitPortabilityProblems': 'false',
          'conditions': [
            [ 'msvs_multi_core_compile', {
              'AdditionalOptions': ['/MP'],
            }],
          ],
        },
        'VCLibrarianTool': {
          'AdditionalOptions': ['/ignore:4221'],
          'conditions': [
            ['target_arch=="x64"', {
              'TargetMachine': '17',  # x64
            }, {
              'TargetMachine': '1',  # ia32
            }],
          ],
        },
        'VCLinkerTool': {
          'GenerateDebugInformation': 'true',
          'MapFileName': '$(OutDir)\\$(TargetName).map',
          'ImportLibrary': '$(OutDir)\\lib\\$(TargetName).lib',
          'FixedBaseAddress': '1',
          'ImageHasSafeExceptionHandlers': 'false',
          # LinkIncremental values:
          #   0 == default
          #   1 == /INCREMENTAL:NO
          #   2 == /INCREMENTAL
          'LinkIncremental': '2',
          # SubSystem values:
          #   0 == not set
          #   1 == /SUBSYSTEM:CONSOLE
          #   2 == /SUBSYSTEM:WINDOWS
          'SubSystem': '1',
        },
      },
    },
  }],  # OS=="win"
],

}
