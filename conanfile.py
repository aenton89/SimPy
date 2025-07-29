from conan import ConanFile


class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("imgui/1.90.5")
        self.requires("glfw/3.3.8")
        self.requires("glad/0.1.36")
        self.requires("implot/0.16")
        self.requires("nlohmann_json/3.12.0")

    def configure(self):
        self.options["imgui/*"].with_glfw = True
        self.options["imgui/*"].with_opengl3 = True

    def build_requirements(self):
        self.tool_requires("cmake/4.0.3")


