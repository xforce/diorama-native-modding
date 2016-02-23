{
  'variables': {
    'wno_array_bounds%': '',
    'enable_backtrace%': 0,
  },
  'target_defaults': {
   'configurations': {
      # Abstract configuration for v8_optimized_debug == 0.
      'DebugBase0': {
        'abstract': 1,
        'msvs_settings': {
          'VCCLCompilerTool': {
            'DebugInformationFormat': '4',
            'Optimization': '0',
			'WarningLevel': '<(debug_warning_level_win)',
            'WarnAsError': 'true',
            'conditions': [
              ['component=="shared_library"', {
                'RuntimeLibrary': '3',  # /MDd
              }, {
                'RuntimeLibrary': '1',  # /MTd
              }],
            ],
          },
          'VCLinkerTool': {
            'LinkIncremental': '2',
            'ImageHasSafeExceptionHandlers': 'false',
          },
        },
        'conditions': [
          ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="netbsd" or \
            OS=="qnx"', {
            'cflags!': [
              '-O3',
              '-O2',
              '-O1',
              '-Os',
            ],
            'cflags': [
              '-fdata-sections',
              '-ffunction-sections',
              '-fPIC',
            ],
          }],
          ['OS=="mac"', {
            'xcode_settings': {
               'GCC_OPTIMIZATION_LEVEL': '0',  # -O0
            },
          }],
        ],
      },  # DebugBase0
      # Abstract configuration for v8_optimized_debug == 1.
      'DebugBase1': {
        'abstract': 1,
        'msvs_settings': {
          'VCCLCompilerTool': {
            'Optimization': '1',
            'InlineFunctionExpansion': '2',
            'EnableIntrinsicFunctions': 'true',
            'FavorSizeOrSpeed': '0',
            'WarningLevel': '<(debug_warning_level_win)',
            'WarnAsError': 'true',
            'StringPooling': 'true',
            'BasicRuntimeChecks': '0',
            'conditions': [
              ['component=="shared_library"', {
                'RuntimeLibrary': '3',  # /MDd
              }, {
                'RuntimeLibrary': '1',  # /MTd
              }],
            ],
          },
          'VCLinkerTool': {
            'LinkIncremental': '2',
            'ImageHasSafeExceptionHandlers': 'false',
          },
        },
        'conditions': [
          ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="netbsd" or \
            OS=="qnx"', {
            'cflags!': [
              '-O0',
              '-O3', # TODO(2807) should be -O1.
              '-O2',
              '-Os',
            ],
            'cflags': [
              '-fdata-sections',
              '-ffunction-sections',
              '-O1', # TODO(2807) should be -O3.
            ],
          }],
          ['OS=="mac"', {
            'xcode_settings': {
               'GCC_OPTIMIZATION_LEVEL': '3',  # -O3
               'GCC_STRICT_ALIASING': 'YES',
            },
          }],
        ],
      },  # DebugBase1
      # Abstract configuration for v8_optimized_debug == 2.
      'DebugBase2': {
        'abstract': 1,
        'msvs_settings': {
          'VCCLCompilerTool': {
            'Optimization': '2',
            'InlineFunctionExpansion': '2',
            'EnableIntrinsicFunctions': 'true',
            'FavorSizeOrSpeed': '0',
			      'WarningLevel': '<(debug_warning_level_win)',
            'WarnAsError': 'true',
            'StringPooling': 'true',
            'BasicRuntimeChecks': '0',
            'conditions': [
              ['component=="shared_library"', {
                'RuntimeLibrary': '3',  #/MDd
              }, {
                'RuntimeLibrary': '1',  #/MTd
              }],
            ],
          },
          'VCLinkerTool': {
            'LinkIncremental': '1',
            'OptimizeReferences': '2',
            'EnableCOMDATFolding': '2',
            'ImageHasSafeExceptionHandlers': 'false',
          },
        },
        'conditions': [
          ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="netbsd" or \
            OS=="qnx"', {
            'cflags!': [
              '-O0',
              '-O1',
              '-Os',
            ],
            'cflags': [
              '-fdata-sections',
              '-ffunction-sections',
            ],
          }],
          ['OS=="mac"', {
            'xcode_settings': {
              'GCC_OPTIMIZATION_LEVEL': '3',  # -O3
              'GCC_STRICT_ALIASING': 'YES',
            },
          }],
        ],
      },  # DebugBase2
      # Common settings for the Debug configuration.
      'DebugBaseCommon': {
        'abstract': 1,
        'defines': [
          'DEBUG',
          'WIN32_LEAN_AND_MEAN',
        ],
        'conditions': [
          ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="netbsd" or \
            OS=="qnx"', {
            'cflags': [ '<(wno_array_bounds)', ],
            'cflags_cc': ['-Woverloaded-virtual',],
          }],
          ['OS=="linux" and enable_backtrace==1', {
            # Support for backtrace_symbols.
            'ldflags': [ '-rdynamic' ],
          }],
        ],
      },  # DebugBaseCommon
      'Debug': {
        'inherit_from': ['DebugBaseCommon'],
        'conditions': [
          ['optimized_debug==0', {
            'inherit_from': ['DebugBase0'],
          }],
          ['optimized_debug==1', {
            'inherit_from': ['DebugBase1'],
          }],
          ['optimized_debug==2', {
            'inherit_from': ['DebugBase2'],
          }],
        ],
        'cflags_cc' : [
            '-std=c++1y',
        ],
      },  # Debug
      'Release': {
        'conditions': [
          ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="netbsd"', {
            'cflags!': [
              '-Os',
            ],
            'cflags': [
              '-fdata-sections',
              '-ffunction-sections',
              '<(wno_array_bounds)',
            ],
          }],
          ['OS=="android"', {
            'cflags!': [
              '-O3',
              '-Os',
            ],
            'cflags': [
              '-fdata-sections',
              '-ffunction-sections',
              '-O2',
            ],
            'cflags_cc' : [
                '-std=c++1y',
            ]
          }],
          ['OS=="mac"', {
            'xcode_settings': {
              'GCC_OPTIMIZATION_LEVEL': '3',  # -O3

              # -fstrict-aliasing.  Mainline gcc
              # enables this at -O2 and above,
              # but Apple gcc does not unless it
              # is specified explicitly.
              'GCC_STRICT_ALIASING': 'YES',
            },
          }],  # OS=="mac"
          ['OS=="win"', {
            'defines': [
              'WIN32_LEAN_AND_MEAN',
              'NDEBUG',
            ],
            'msvs_settings': {
              'VCCLCompilerTool': {
                'Optimization': '2',
                'InlineFunctionExpansion': '2',
                'EnableIntrinsicFunctions': 'true',
                'FavorSizeOrSpeed': '0',
                'StringPooling': 'true',
                #'WholeProgramOptimization': 'true',  # /GL
                'conditions': [
                  ['component=="shared_library"', {
                    'RuntimeLibrary': '2',  #/MD
                  }, {
                    'RuntimeLibrary': '0',  #/MT
                  }],
                ],
              },
              'VCLinkerTool': {
                'LinkIncremental': '1',
                'OptimizeReferences': '2',
                'EnableCOMDATFolding': '2',
                'ImageHasSafeExceptionHandlers': 'false',
                #'LinkTimeCodeGeneration': '1',       # /LTCG
              },
            },
          }],  # OS=="win"
        ],  # conditions
      },  # Release
    },  # configurations
  }, # target_defaults
}
