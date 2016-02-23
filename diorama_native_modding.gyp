{
    'variables' : {
        'diorama_native_modding_target_type%': 'executable',
        'deps_path%': 'deps',
    },
    'targets': [
        {
            'target_name': 'diorama_native_modding',
            'type': '<(diorama_native_modding_target_type)',
            'defines': [
                'NOMINMAX',
            ],
            'include_dirs': [
                '.',
            ],
            'direct_dependent_settings': {
                'include_dirs': [
                    '.',
                ],
                # TODO ADD LIBRARIES FOR WINDOWS BUILDS
            },
            'sources': [
                #Source files
                'src/main.cpp',
                
                'src/peloader.h',
                'src/peloader.cpp',
                
                'src/dummies.cpp',
                
                'src/hooking/hooking.h',
                'src/hooking/hooking.cpp',
                
                'src/hooking/hooking_patterns.h',
                'src/hooking/hooking_patterns.cpp',
                 
                'src/vector.h',
                'src/matrix.h', 
            ],
            
            'conditions': [
                ['OS=="win"', {
                    'defines': [
                        '_WINSOCK_DEPRECATED_NO_WARNINGS',
                    ],
                    'link_settings':  {
                        'libraries': [ '-lwinmm.lib', '-lws2_32.lib' ],
                    },
                    'msvs_settings': {
                        'VCLinkerTool': {
                            'ImageHasSafeExceptionHandlers': 'false',
                            'LargeAddressAware': '2',
						    'BaseAddress': '0x0750000',
							'AdditionalOptions': [
							    '/IGNORE:4254',
							    '/DYNAMICBASE:NO', 
							    '/SAFESEH:NO', 
          				    ],
                        },
                    },
                }],
            ],
        },
    ]
}
