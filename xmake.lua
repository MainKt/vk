add_rules("mode.debug", "mode.release")

set_languages("c++23")

if is_mode("debug") then
	set_policy("build.sanitizer.address", true)

	if is_plat("windows") and is_host("windows") then
		add_cxxflags("/Zi", { tools = { "cl" } })
	else
		set_policy("build.sanitizer.undefined", true)
		add_cxxflags("-g")
	end

	add_rules("plugin.compile_commands.autoupdate")
end

if is_plat("windows") and is_host("windows") then
	add_cxxflags("/W4", { tools = { "cl" } })
else
	add_cxxflags("-Wall -Wextra -Wpedantic -Wshadow -Wunused")
end

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
