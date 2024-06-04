{
	'targets': [
		{
			'target_name': 'node_bzip2',
			"actions": [
				{
					"action_name": "configure_bzip2",
					"inputs": [],
					"outputs": ["<(INTERMEDIATE_DIR)/bzip2"],
					"action": [
						"cmake",
						"-S",
						"lib/bzip2",
						"-B",
						"<(INTERMEDIATE_DIR)/bzip2",
						"-DCMAKE_BUILD_TYPE=Release",
						"-DENABLE_LIB_ONLY=1",
						"-DENABLE_SHARED_LIB=0",
						"-DENABLE_STATIC_LIB=1"
						"-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL"
					]
				},
				{
					"action_name": "build_bzip2",
					"inputs": ["<(INTERMEDIATE_DIR)/bzip2"],
					"outputs": ["<(INTERMEDIATE_DIR)/bzip2/CMakeLists.txt"],
					"action": [
						"cmake",
						"--build",
						"<(INTERMEDIATE_DIR)/bzip2",
						"--config=Release"
					]
				}
			],
			'sources': [
				'src/node_bzip2_wrapper.cpp'
			],
			'include_dirs': [
				'<!(node -e "require(\'nan\')")',
				'lib/bzip2'
			],
			'cflags!': [ '-fno-exceptions' ],
			'cflags_cc!': [ '-fno-exceptions' ],
			'msvs_settings': {
				'VCCLCompilerTool': {
					'ExceptionHandling': 1,
					'RuntimeLibrary': "MultiThreadedDLL",
					'AdditionalOptions': ['/MD']
				}
			},
			'conditions': [
				['OS=="mac"', {
					'xcode_settings': {
						'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
					}
				}],

				['OS=="win"', {
					'defines': [
						'_HAS_EXCEPTIONS=1'
					]
				}],

				["OS=='win'", {
					"libraries": [
						"<(INTERMEDIATE_DIR)/bzip2/Release/bz2_static.lib"
					]
				}, {
					"libraries": [
						"<(INTERMEDIATE_DIR)/bzip2/libbz2_static.a"
					]
				}]
			]
		}
	]
}