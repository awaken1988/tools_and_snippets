#include <webgpu/webgpu.h>
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include <iostream>
#include <string>
#include <vector>

const static std::vector<float> vertices = {
		   -0.5, -0.5,
		   +0.5, -0.5,
		   +0.0, +0.5,
};

//shader
const char* shaderSource = R"(
@vertex
fn vs_main(@location(0) in_vertex_position: vec2f) -> @builtin(position) vec4f {
	return vec4f(in_vertex_position, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
	return vec4f(0.0, 0.4, 1.0, 1.0);
}
)";  

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
	WGPUTextureFormat m_swapChainFormat;
	WGPUSwapChain m_swapChain;
	WGPURenderPipeline m_pipeline;
	WGPUBuffer m_vertexBuffer;
	WGPUBufferDescriptor m_vertexBufferDescriptor = {};

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
			m_swapChainFormat = WGPUTextureFormat_BGRA8Unorm; //wgpuSurfaceGetPreferredFormat(m_surface, m_adapter);

			WGPUSwapChainDescriptor swapChainDesc = {};
			swapChainDesc.nextInChain = nullptr;
			swapChainDesc.width = WINDOW_WIDTH;
			swapChainDesc.height = WINDOW_HEIGHT;
			swapChainDesc.format = m_swapChainFormat;
			swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
			swapChainDesc.presentMode = WGPUPresentMode_Fifo;

			m_swapChain = wgpuDeviceCreateSwapChain(m_device, m_surface, &swapChainDesc);
			std::cout << "Swapchain: " << m_swapChain << std::endl;
		}

		//pipeline
		{
			//shader
			WGPUShaderModuleWGSLDescriptor shaderCodeDesc{};
			shaderCodeDesc.chain.next = nullptr;
			shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
			shaderCodeDesc.code = shaderSource;
			
			WGPUShaderModuleDescriptor shaderDesc{};
			shaderDesc.nextInChain = &shaderCodeDesc.chain;
			WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(m_device, &shaderDesc);

			//pipeline
			WGPURenderPipelineDescriptor pipelineDesc = {};
			pipelineDesc.nextInChain = nullptr;

			WGPUVertexAttribute vertexAttribute = {};
			vertexAttribute.format = WGPUVertexFormat_Float32x2;
			vertexAttribute.offset = 0;
			vertexAttribute.shaderLocation = 0;

			WGPUVertexBufferLayout vertexBufferLayout = {};
			vertexBufferLayout.attributeCount = 1;
			vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;
			vertexBufferLayout.arrayStride = 2 * sizeof(float);
			vertexBufferLayout.attributes = &vertexAttribute;
			vertexBufferLayout.attributeCount = 1;

			pipelineDesc.vertex.bufferCount = 1;
			pipelineDesc.vertex.buffers = &vertexBufferLayout;
			pipelineDesc.vertex.module = shaderModule;
			pipelineDesc.vertex.entryPoint = "vs_main";
			pipelineDesc.vertex.constantCount = 0;
			pipelineDesc.vertex.constants = nullptr;

			pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
			pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
			pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
			pipelineDesc.primitive.cullMode = WGPUCullMode_None;

			WGPUFragmentState fragmentState = {};
			fragmentState.nextInChain = nullptr;
			pipelineDesc.fragment = &fragmentState;
			fragmentState.module = shaderModule;
			fragmentState.entryPoint = "fs_main";
			fragmentState.constantCount = 0;
			fragmentState.constants = nullptr;

			WGPUBlendState blendState{};
			blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
			blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
			blendState.color.operation = WGPUBlendOperation_Add;
			blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
			blendState.alpha.dstFactor = WGPUBlendFactor_One;
			blendState.alpha.operation = WGPUBlendOperation_Add;

			WGPUColorTargetState colorTarget{};
			colorTarget.nextInChain = nullptr;
			colorTarget.format = m_swapChainFormat;
			colorTarget.blend = &blendState;
			colorTarget.writeMask = WGPUColorWriteMask_All; // We could write to only some of the color channels.

			fragmentState.targetCount = 1;
			fragmentState.targets = &colorTarget;

			pipelineDesc.depthStencil = nullptr;

			pipelineDesc.multisample.count = 1;
			pipelineDesc.multisample.mask = ~0u;
			pipelineDesc.multisample.alphaToCoverageEnabled = false;


			//pipelineDesc.fragment = &fragmentState;  
			
			pipelineDesc.layout = nullptr;

			m_pipeline = wgpuDeviceCreateRenderPipeline(m_device, &pipelineDesc);

			//TODO: https://eliemichel.github.io/LearnWebGPU/basic-3d-rendering/input-geometry/index.html
		}

		//create vertex buffer
		{
			WGPUBufferDescriptor m_vertexBufferDescriptor = {};
			m_vertexBufferDescriptor.nextInChain = nullptr;
			m_vertexBufferDescriptor.label = "My first Vertex Buffer";
			m_vertexBufferDescriptor.size = vertices.size() * sizeof(float);
			m_vertexBufferDescriptor.mappedAtCreation = false;
			m_vertexBufferDescriptor.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;

			m_vertexBuffer = wgpuDeviceCreateBuffer(m_device, &m_vertexBufferDescriptor);

			wgpuQueueWriteBuffer(m_queue, m_vertexBuffer, 0, vertices.data(), m_vertexBufferDescriptor.size);
		}

	}

	bool isSecond = false;
	void run() {
		while (!glfwWindowShouldClose(m_window)) {
			glfwPollEvents();
			
			WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(m_swapChain);
			if (!nextTexture) {
				std::cerr << "Cannot acquire next swap chain texture" << std::endl;
				break;
			}
			//std::cout << "nextTexture: " << nextTexture << std::endl;

			WGPUCommandEncoderDescriptor commandEncoderDesc = {};
			commandEncoderDesc.nextInChain = nullptr;
			commandEncoderDesc.label = "Command Encoder";
			WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_device, &commandEncoderDesc);

			// Describe a render pass, which targets the texture view
			WGPURenderPassDescriptor renderPassDesc = {};
			renderPassDesc.nextInChain = nullptr;

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

			// Create a render pass. We end it immediately because we use its built-in
			// mechanism for clearing the screen when it begins (see descriptor).
			WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
			wgpuRenderPassEncoderSetPipeline(renderPass, m_pipeline);
			wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, m_vertexBuffer, 0, m_vertexBufferDescriptor.size);
			wgpuRenderPassEncoderDraw(renderPass, vertices.size()/2, 1, 0, 0);
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