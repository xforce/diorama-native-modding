{
    'variables' : {
        'diorama_native_modding_target_type%': 'executable',
        'deps_path%': 'deps',
    },

    'targets': [
        {
            'target_name': 'dio_shared_lib',
            'type': 'static_library',
            'defines': [
                'NOMINMAX',
                '_ITERATOR_DEBUG_LEVEL=0',
            ],
            'include_dirs': [
                '.',
            ],
            'direct_dependent_settings': {
                'include_dirs': [
                    '.',
                ],
                'defines': [
                    'NOMINMAX',
                    '_ITERATOR_DEBUG_LEVEL=0',
                ],
                # TODO ADD LIBRARIES FOR WINDOWS BUILDS
            },
            'sources' : [
                'src/vector.h',
                'src/matrix.h', 
            ],
        },

        {
            'target_name': 'dio_client_lib',
            'type': 'static_library',
            'defines': [
                'NOMINMAX',
                '_ITERATOR_DEBUG_LEVEL=0',
            ],
            'include_dirs': [
                '.',
            ],
            'direct_dependent_settings': {
                'include_dirs': [
                    '.',
                ],
                'defines': [
                    'NOMINMAX',
                    '_ITERATOR_DEBUG_LEVEL=0',
                ],
                # TODO ADD LIBRARIES FOR WINDOWS BUILDS
            },
        },

        {
            'target_name': 'dio_server_lib',
            'type': 'static_library',
            'defines': [
                'NOMINMAX',
                '_ITERATOR_DEBUG_LEVEL=0',
            ],
            'include_dirs': [
                '.',
            ],
            'direct_dependent_settings': {
                'include_dirs': [
                    '.',
                ],
                'defines': [
                    'NOMINMAX',
                    '_ITERATOR_DEBUG_LEVEL=0',
                ],
                # TODO ADD LIBRARIES FOR WINDOWS BUILDS
            },

        },

        {
            'target_name': 'hooking_lib',
            'type': 'static_library',
            'defines': [
                'NOMINMAX',
                '_ITERATOR_DEBUG_LEVEL=0',
            ],
            'include_dirs': [
                '.',
            ],
            'direct_dependent_settings': {
                'include_dirs': [
                    '.',
                ],
                'defines': [
                    'NOMINMAX',
                    '_ITERATOR_DEBUG_LEVEL=0',
                ],
                # TODO ADD LIBRARIES FOR WINDOWS BUILDS
            },
            'sources':[
                'src/hooking/hooking.h',
                'src/hooking/hooking.cpp',
                
                'src/hooking/hooking_patterns.h',
                'src/hooking/hooking_patterns.cpp',   
            ],
        },

        {
            'target_name': 'loader_lib',
            'type': 'static_library',
            'defines': [
                'NOMINMAX',
                '_ITERATOR_DEBUG_LEVEL=0',
            ],
            'include_dirs': [
                'src/loader/win32',
            ],
            'direct_dependent_settings': {
                'include_dirs': [
                    'src/loader/win32',
                ],
                'defines': [
                    'NOMINMAX',
                    '_ITERATOR_DEBUG_LEVEL=0',
                ],
                # TODO ADD LIBRARIES FOR WINDOWS BUILDS
            },
            # TODO(xforce): Condition shit
            'sources' : [
                'src/loader/win32/peloader.h',
                'src/loader/win32/peloader.cpp',
            ],
        },

        {
            'target_name': 'diorama_native_modding',
            'type': '<(diorama_native_modding_target_type)',
            'dependencies' : [
                #'dio_shared_lib',
                #'dio_client_lib',
                'loader_lib',
                'hooking_lib',
            ],
            'sources': [
                #Source files
                'src/main.cpp',
                'src/dummies.cpp',
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
