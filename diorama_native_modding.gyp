{
    # TODO(xforce): Clean this up, split it in multiple files
    
    
    'variables' : {
        'diorama_native_modding_target_type%': 'executable',
        'deps_path%': 'deps',
    },

    'targets': [
        {
            'target_name': 'dio_shared_lib',
            'type': 'static_library',
            
             'dependencies' : [
                'hooking_lib',
            ],
            
            'defines': [
                'NOMINMAX',
                '_ITERATOR_DEBUG_LEVEL=0',
            ],
            
            
            'include_dirs': [
                '.',
                'src/dio/shared',
            ],
            'direct_dependent_settings': {
                'include_dirs': [
                    '.',
                    'src/dio/shared',
                    'src',
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
                'src/dio/shared/allocator.h',
                'src/dio/shared/dio_std_types.h',  
            ],
             'conditions':[
                ['target_arch=="x64"', {
                    'sources' : [
                        'src/dio/shared/impl/win32_x64/allocator.cpp',
                    ],
                }],
            ],
        },

        {
            'target_name': 'dio_client_lib',
            'type': 'static_library',
            'dependencies' : [
                'hooking_lib',
                'dio_shared_lib',
            ],
            'defines': [
                'NOMINMAX',
                '_ITERATOR_DEBUG_LEVEL=0',
            ],
            'include_dirs': [
                '.',
                'src/dio/client/',
            ],
            'sources' : [
              'src/dio/client/events/event.h',
              'src/dio/client/events/toserver_chat.h',
              'src/dio/client/events/toserver_playerstate.h',
              'src/dio/client/networkingclient.h',
              
            ],
            
            'conditions':[
                ['target_arch=="x64"', {
                    'sources' : [
                        'src/dio/client/impl/win32_x64/events/event.cpp',
                        'src/dio/client/impl/win32_x64/events/toserver_playerstate.cpp',
                        'src/dio/client/impl/win32_x64/events/toserver_chat.cpp',
                        'src/dio/client/impl/win32_x64/networkingclient.cpp',
                    ],
                }],
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
            'type': 'shared_library',
            'defines': [
                'NOMINMAX',
                '_ITERATOR_DEBUG_LEVEL=0',
            ],
            'include_dirs': [
                '.',
                'src/hooking/',
            ],
            'direct_dependent_settings': {
                'include_dirs': [
                    '.',
                    'src/hooking/',
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
            'target_name': 'diorama_native_modding_client',
            'type': '<(diorama_native_modding_target_type)',
            'dependencies' : [
                'dio_shared_lib',
                'dio_client_lib',
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
        
        {
            'target_name': 'diorama_native_modding_server',
            'type': '<(diorama_native_modding_target_type)',
            'dependencies' : [
                'dio_shared_lib',
                'dio_server_lib',
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
