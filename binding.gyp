{
	"targets": [
		{
			"target_name": "node_bzip2",
			"sources": [
				"src/node_bzip2_wrapper.cpp"
			],
			"include_dirs": [
        		"<!(node -e \"require('nan')\")",
				"lib/bzip2"
			]
		}
	]
}