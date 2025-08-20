from conan import ConanFile


class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("imgui/1.90.5")
        self.requires("glfw/3.3.8")
        # + glad, coś nowego
        self.requires("glad/0.1.36")
        # implot dla wykresów
        self.requires("implot/0.16")
        # dla serializacji
        self.requires("nlohmann_json/3.12.0")
        # dla obrazka
        self.requires("stb/cci.20240531")

    def configure(self):
        self.options["imgui/*"].with_glfw = True
        self.options["imgui/*"].with_opengl3 = True

    def build_requirements(self):
        self.tool_requires("cmake/3.31.6")