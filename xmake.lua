add_rules("mode.debug", "mode.release")

set_languages("c++23")
set_policy("build.sanitizer.address", true)
set_policy("build.sanitizer.undefined", true)
add_rules("plugin.compile_commands.autoupdate")
add_cxxflags("-Wall -Wextra -Wpedantic -Wshadow -g")

add_defines("VK_NO_PROTOTYPES")
add_defines("GLFW_INCLUDE_VULKAN")

local deps = {
	"glfw",
	"vulkan",
	"vulkansdk",
	"vulkan-headers",
	"vulkan-loader",
	"vulkan-hpp",
	"vulkan-memory-allocator",
	"vulkan-utility-libraries",
	"vulkan-validationlayers",
	"glm",
	"imgui"
}

add_requires(table.unpack(deps))
	target("main")
	set_kind("binary")
	add_files("src/*.cpp")
	add_packages(table.unpack(deps))
