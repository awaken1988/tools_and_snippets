#include <webgpu/webgpu.h>
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void onAdapterRequestEnded(
	WGPURequestAdapterStatus status, // a success status
	WGPUAdapter adapter, // the returned adapter
	char const* message, // error message, or nullptr
	void* userdata // custom user data, as provided when requesting the adapter
);

void onDeviceRequestEnded(
	WGPURequestDeviceStatus status,
	WGPUDevice device,
	char const* message,
	void* pUserData);

void onErrorDevice(
	WGPUErrorType type,
	char const* message,
	void* pUserData);

void onQueueWorkDone(
	WGPUQueueWorkDoneStatus status,
	void* pUserData);

struct Render
{
	static constexpr int WINDOW_WIDTH = 1024;
	static constexpr int WINDOW_HEIGHT = 768;

	GLFWwindow* m_window;
	WGPUInstance m_instance;
	WGPUAdapter m_adapter;
	WGPUSurface m_surface;
	vector<WGPUFeatureName> m_features;
	WGPUDevice m_device;
	WGPUQueue m_queue;
	WGPUCommandEncoder m_encoder;
	WGPUSwapChain m_swapChain;

	Render() {
		if (!glfwInit())
			throw string{"glfwInit"};

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Learn WebGPU", NULL, NULL);

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

		//request adapter
		{
			const auto featureCount = wgpuAdapterEnumerateFeatures(m_adapter, nullptr);
			m_features.resize(featureCount);
			wgpuAdapterEnumerateFeatures(m_adapter, m_features.data());
			for (const auto& iFeature : m_features) {
				cout << "Feature: " << iFeature << endl;
			}
		}

		//request device
		{
			WGPUDeviceDescriptor descriptor = {};
			descriptor.label = "My first WebGPU device";
			descriptor.requiredFeaturesCount = 0;
			descriptor.requiredLimits = nullptr;
			descriptor.defaultQueue.nextInChain = nullptr;
			descriptor.defaultQueue.nextInChain = nullptr;
			wgpuAdapterRequestDevice(m_adapter, &descriptor, &onDeviceRequestEnded, this);
			wgpuDeviceSetUncapturedErrorCallback(m_device, &onErrorDevice, this); 
		}
		
		//queue
		{
			m_queue = wgpuDeviceGetQueue(m_device);
			wgpuQueueOnSubmittedWorkDone(m_queue, 0, &onQueueWorkDone, this);
		}

		//command encoder
		{
			WGPUCommandEncoderDescriptor encoderDesc = {};
			encoderDesc.nextInChain = nullptr;
			encoderDesc.label = "My command encoder";
			m_encoder = wgpuDeviceCreateCommandEncoder(m_device, &encoderDesc);
		}

		//swapchain
		{
			WGPUTextureFormat swapChainFormat = WGPUTextureFormat_BGRA8Unorm; //wgpuSurfaceGetPreferredFormat(m_surface, m_adapter);

			WGPUSwapChainDescriptor swapChainDesc = {};
			swapChainDesc.nextInChain = nullptr;
			swapChainDesc.width = WINDOW_WIDTH;
			swapChainDesc.height = WINDOW_HEIGHT;
			swapChainDesc.format = swapChainFormat;
			swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
			swapChainDesc.presentMode = WGPUPresentMode_Fifo;

			m_swapChain = wgpuDeviceCreateSwapChain(m_device, m_surface, &swapChainDesc);
			std::cout << "Swapchain: " << m_swapChain << std::endl;
		}

	}

	bool isSecond = false;
	void run() {
		while (!glfwWindowShouldClose(m_window)) {
			glfwPollEvents();
			
			WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(m_swapChain);.
			if (!nextTexture) {
				std::cerr << "Cannot acquire next swap chain texture" << std::endl;
				break;
			}
			std::cout << "nextTexture: " << nextTexture << std::endl;

			WGPUCommandEncoderDescriptor commandEncoderDesc = {};
			commandEncoderDesc.nextInChain = nullptr;
			commandEncoderDesc.label = "Command Encoder";
			WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_device, &commandEncoderDesc);

			// Describe a render pass, which targets the texture view
			WGPURenderPassDescriptor renderPassDesc = {};

			WGPURenderPassColorAttachment renderPassColorAttachment = {};
			renderPassColorAttachment.view = nextTexture;
			renderPassColorAttachment.resolveTarget = nullptr;
			renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
			renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
			renderPassColorAttachment.clearValue = WGPUColor{ 0.9, 0.1, 0.2, 1.0 };
			renderPassDesc.colorAttachmentCount = 1;
			renderPassDesc.colorAttachments = &renderPassColorAttachment;

			// No depth buffer for now
			renderPassDesc.depthStencilAttachment = nullptr;

			// We do not use timers for now neither
			renderPassDesc.timestampWriteCount = 0;
			renderPassDesc.timestampWrites = nullptr;

			renderPassDesc.nextInChain = nullptr;

			// Create a render pass. We end it immediately because we use its built-in
			// mechanism for clearing the screen when it begins (see descriptor).
			WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
			wgpuRenderPassEncoderEnd(renderPass);

			wgpuTextureViewRelease(nextTexture);

			WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
			cmdBufferDescriptor.nextInChain = nullptr;
			cmdBufferDescriptor.label = "Command buffer";
			WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
			wgpuQueueSubmit(m_queue, 1, &command);

			// We can tell the swap chain to present the next texture.
			wgpuSwapChainPresent(m_swapChain);
		}
	}

	~Render() {
		wgpuSwapChainRelease(m_swapChain);
		wgpuDeviceRelease(m_device); 
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

void onDeviceRequestEnded(
	WGPURequestDeviceStatus status,
	WGPUDevice device,
	char const* message,
	void* pUserData)
{
	if (status != WGPURequestDeviceStatus_Success) {
		throw string{"cannot create device"};
	}
	else {
		cout << "device created" << endl;
	}

	auto* render = reinterpret_cast<Render*>(pUserData);
	render->m_device = device;
}

void onErrorDevice(
	WGPUErrorType type,
	char const* message,
	void* pUserData)
{
	cout << "[ERROR] " << message << endl;
}

void onQueueWorkDone(
	WGPUQueueWorkDoneStatus status,
	void* pUserData)
{
	cout << "queue work done" << endl;     
}


int main() 
{	
	try {
		Render render;
		render.run();
	}
	catch (const string& e) {
		cout << "[ERROR] " << e << endl;
	}

	return 0;
} 