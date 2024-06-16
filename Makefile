.PHONY: format check-format

format:
	clang-format -i src/*.{cpp,h} lib/ANSI_core/*.{cpp,h} lib/TANSI_platform_teensy41/*.{cpp,h}

check-format:
	clang-format -n src/*.{cpp,h} lib/ANSI_core/*.{cpp,h} lib/TANSI_platform_teensy41/*.{cpp,h}
