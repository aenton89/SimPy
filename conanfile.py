from conan import ConanFile


class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("imgui/1.89.9")
        self.requires("glfw/3.3.8")

    def configure(self):
        self.options["imgui/*"].with_glfw = True
        self.options["imgui/*"].with_opengl3 = True

    def build_requirements(self):
        self.tool_requires("cmake/3.31.6")