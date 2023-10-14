#include <webgpu/webgpu.h>
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include <iostream>
#include <string>
using namespace std;

void onAdapterRequestEnded(
	WGPURequestAdapterStatus status, // a success status
	WGPUAdapter adapter, // the returned adapter
	char const* message, // error message, or nullptr
	void* userdata // custom user data, as provided when requesting the adapter
);

struct Render
{
	GLFWwindow* m_window;
	WGPUInstance m_instance;
	WGPUAdapter m_adapter;
	WGPUSurface m_surface;

	Render() {
		if (!glfwInit())
			throw string{"glfwInit"};

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_window = glfwCreateWindow(640, 480, "Learn WebGPU", NULL, NULL);

		//instance
		{
			WGPUInstanceDescriptor instanceDescriptor = {};
			instanceDescriptor.nextInChain = nullptr;

			m_instance = wgpuCreateInstance(&instanceDescriptor);
			if (!m_instance)
				throw string{"wgpuCreateInstance"};

		}

		//adapter
		{
			m_surface = glfwGetWGPUSurface(m_instance, m_window);

			WGPURequestAdapterOptions options = {};
			options.nextInChain = nullptr;
			options.compatibleSurface = m_surface; 
			wgpuInstanceRequestAdapter(m_instance, &options, &onAdapterRequestEnded, this);
		}

		//TODO: https://eliemichel.github.io/LearnWebGPU/getting-started/the-adapter.html
		

	}

	void run() {
		while (!glfwWindowShouldClose(m_window)) {
			// Check whether the user clicked on the close button (and any other
			// mouse/key event, which we don't use so far)
			glfwPollEvents();
		}
	}

	~Render() {
		wgpuAdapterRelease(m_adapter);
		wgpuInstanceRelease(m_instance);
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

};

void onAdapterRequestEnded(
	WGPURequestAdapterStatus status, // a success status
	WGPUAdapter adapter, // the returned adapter
	char const* message, // error message, or nullptr
	void* userdata // custom user data, as provided when requesting the adapter
)
{
	if (message != nullptr)
		throw string{message};
	cout << "adapter" << endl; 

	auto* render = reinterpret_cast<Render*>(userdata);
	render->m_adapter = adapter; 

}


int main() 
{	
	try {
		Render render;
		//render.run();
	}
	catch (const string& e) {
		cout << "[ERROR] " << e << endl;
	}

	return 0;
} 