#define IMGUI_DEFINE_MATH_OPERATORS
#define _CRT_SECURE_NO_WARNINGS
#include "overlay.hpp"
#include "../../dependencies/imgui/imgui.h"
#include "../../dependencies/imgui/backends/imgui_impl_win32.h"
#include "../../dependencies/imgui/backends/imgui_impl_dx11.h"
#include "../../dependencies/imgui/imgui_internal.h"
#include <dwmapi.h>
#include <filesystem>
#include <vector>
#include <string>
#include <d3d11.h>
#include "../rbx\storage/storage.hpp"
#include <functional>
#include <random>
#include <thread>
#include "../../dependencies/imgui/TextEditor/TextEditor.h"
#include "../../dependencies/zstd/zstd.h"
#include "../../dependencies/xxhash.h"
#include "..\..\dependencies/Luau/Compiler.h"
#include "..\..\dependencies/Luau/BytecodeBuilder.h"
#include "..\..\dependencies/Luau/BytecodeUtils.h"

ID3D11Device* gui::overlay::d3d11_device = nullptr;

ID3D11DeviceContext* gui::overlay::d3d11_device_context = nullptr;

IDXGISwapChain* gui::overlay::dxgi_swap_chain = nullptr;

ID3D11RenderTargetView* gui::overlay::d3d11_render_target_view = nullptr;

namespace cuminaniggaass {
	class bytecode_encoder_t : public Luau::BytecodeEncoder {
		inline void encode(uint32_t* data, size_t count) override {
			for (auto i = 0u; i < count;) {
				auto& opcode = *reinterpret_cast<uint8_t*>(data + i);
				i += Luau::getOpLength(LuauOpcode(opcode));

				opcode *= 227;
			}
		}
	};

	Luau::CompileOptions compile_options;

	std::string compile(std::string source) {
		if (compile_options.debugLevel != 2) {
			compile_options.debugLevel = 2;
			compile_options.optimizationLevel = 2;
		}

		static auto encoder = bytecode_encoder_t();

		std::string bytecode = Luau::compile(
			source,
			{},
			{}, &encoder
		);

		return bytecode;
	}


	std::vector<char> compress_jest(std::string bytecode, size_t& byte_size) {
		const auto data_size = bytecode.size();
		const auto max_size = ZSTD_compressBound(data_size);
		auto buffer = std::vector<char>(max_size + 8);

		strcpy_s(&buffer[0], buffer.capacity(), "RSB1");
		memcpy_s(&buffer[4], buffer.capacity(), &data_size, sizeof(data_size));

		const auto compressed_size = ZSTD_compress(&buffer[8], max_size, bytecode.data(), data_size, ZSTD_maxCLevel());
		if (ZSTD_isError(compressed_size))
			throw std::runtime_error("Failed to compress the bytecode.");

		const auto size = compressed_size + 8;
		const auto key = XXH32(buffer.data(), size, 42u);
		const auto bytes = reinterpret_cast<const uint8_t*>(&key);

		for (auto i = 0u; i < size; ++i)
			buffer[i] ^= bytes[i % 4] + i * 41u;

		byte_size = size;

		return buffer;
	}
}

bool gui::overlay::init = false;
static bool is_dragging = false;
static ImVec2 drag_offset;

void HandleDragging(ImGuiWindow* window) {
	if (ImGui::IsMouseClicked(0)) {
		if (ImGui::IsItemHovered() && !ImGui::IsItemActive() && !is_dragging) {
			is_dragging = true;
			drag_offset = ImVec2(ImGui::GetMousePos().x - window->Pos.x, ImGui::GetMousePos().y - window->Pos.y);
		}
	}

	if (is_dragging) {
		if (ImGui::IsMouseDown(0)) {
			ImVec2 new_pos = ImVec2(ImGui::GetMousePos().x - drag_offset.x, ImGui::GetMousePos().y - drag_offset.y);
			window->Pos = new_pos;
		}
		else {
			is_dragging = false;
		}
	}
}

int CountLines(const std::string& text) {
	int lines = 1;
	for (char c : text) {
		if (c == '\n') {
			lines++;
		}
	}
	return lines;
}

bool syntaxhighlighting = false;

void gui::overlay::render()
{

	ImGui_ImplWin32_EnableDpiAwareness();

	WNDCLASSEX wc;
	wc.cbClsExtra = NULL;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.cbWndExtra = NULL;
	wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpfnWndProc = window_proc;
	wc.lpszClassName = TEXT("gex");
	wc.lpszMenuName = nullptr;
	wc.style = CS_VREDRAW | CS_HREDRAW;

	RegisterClassEx(&wc);
	const HWND hw = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE, wc.lpszClassName, TEXT("gex"),
		WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, wc.hInstance, nullptr);

	SetLayeredWindowAttributes(hw, 0, 255, LWA_ALPHA);
	const MARGINS margin = { -1 };
	DwmExtendFrameIntoClientArea(hw, &margin);

	if (!create_device_d3d(hw))
	{
		cleanup_device_d3d();
		UnregisterClass(wc.lpszClassName, wc.hInstance);
		return;
	}

	ShowWindow(hw, SW_SHOW);
	UpdateWindow(hw);

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui::GetIO().IniFilename = nullptr;

	ImGui_ImplWin32_Init(hw);
	ImGui_ImplDX11_Init(d3d11_device, d3d11_device_context);

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.Fonts->Build();


	const ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	init = true;

	bool draw = false;
	bool done = false;

	bool check = false;

	std::vector<std::string> consoleLog; // dont delete this!!
	TextEditor editor;

	editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());

	editor.SetText(R"(print('gex on top! (Made by now and http2'))");

	while (!done)
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				done = true;
			}
		}

		if (done)
			break;

		move_window(hw);

		if (check == true) {
			if ((GetAsyncKeyState(VK_END) & 1))
				draw = !draw;
			check = !check;
		}
		else {
			check = !check;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		{
			if (GetForegroundWindow() == FindWindowA(0, "Roblox") || GetForegroundWindow() == hw)
			{
				ImGui::Begin(("overlay"), nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);
				{
					ImGui::End();
				}

				if (draw)
				{

					ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;

					ImGui::Begin("gex (.gg/gex)", nullptr, window_flags);

					HandleDragging(ImGui::GetCurrentWindow());
					static char texte[1024 * 1024] = "";
					if (ImGui::Button("Execute")) {
						auto jestglobals = storage::jestglobals;
						auto Holder = jestglobals.findfirstchild("Holder").ObjectValue();
						auto Exec = jestglobals.findfirstchild("Exec");

						Holder.modulebypassi();
						std::string text;

						if (syntaxhighlighting == true) {
							text = editor.GetText();
						}
						else if (syntaxhighlighting == false) {
							text = texte;
						}

						size_t insertcum;
						auto bytes = cuminaniggaass::compress_jest(cuminaniggaass::compile(std::string("return function(...)\n" + text + "\nend")), insertcum);
						Holder.SetBytecode(bytes, insertcum);
						Exec.SetBoolValue(true);
					}
					ImGui::SameLine();
					ImGui::Checkbox("Syntax Highlighting", &syntaxhighlighting);

					if (syntaxhighlighting == true) {
						editor.Render("TextEditor");
					}
					else {
						ImGui::InputTextMultiline("##text", texte, sizeof(texte),
							ImVec2(-1, 200),
							ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CtrlEnterForNewLine);

					}
					ImGui::End();
				}
			}

				SetWindowDisplayAffinity(hw, WDA_NONE);

			if (draw)
			{
				SetWindowLong(hw, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
			}
			else
			{
				SetWindowLong(hw, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
			}

			ImGui::EndFrame();
			ImGui::Render();

			const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
			d3d11_device_context->OMSetRenderTargets(1, &d3d11_render_target_view, nullptr);
			d3d11_device_context->ClearRenderTargetView(d3d11_render_target_view, clear_color_with_alpha);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

				dxgi_swap_chain->Present(1, 0);
		}
	}

	init = false;

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	cleanup_device_d3d();
	DestroyWindow(hw);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
}

bool gui::overlay::fullsc(HWND windowHandle)
{
	MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
	if (GetMonitorInfo(MonitorFromWindow(windowHandle, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
	{
		RECT windowRect;
		if (GetWindowRect(windowHandle, &windowRect))
		{
			return windowRect.left == monitorInfo.rcMonitor.left
				&& windowRect.right == monitorInfo.rcMonitor.right
				&& windowRect.top == monitorInfo.rcMonitor.top
				&& windowRect.bottom == monitorInfo.rcMonitor.bottom;
		}
	}
}

void gui::overlay::move_window(HWND hw)
{
	HWND target = FindWindowA(0, ("Roblox"));
	HWND foregroundWindow = GetForegroundWindow();

	if (target != foregroundWindow && hw != foregroundWindow)
	{
		MoveWindow(hw, 0, 0, 0, 0, true);
	}
	else
	{
		RECT rect;
		GetWindowRect(target, &rect);

		int rsize_x = rect.right - rect.left;
		int rsize_y = rect.bottom - rect.top;

		if (fullsc(target))
		{
			rsize_x += 16;
			rsize_y -= 24;

			MoveWindow(hw, rect.left, rect.top, rsize_x, rsize_y, TRUE);
		}
		else
		{
			rsize_y -= 63;
			rect.left += 8;
			rect.top += 31;
		}

		MoveWindow(hw, rect.left, rect.top, rsize_x, rsize_y, TRUE);
	}
}

bool gui::overlay::create_device_d3d(HWND hw)
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hw;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	const UINT create_device_flags = 0;
	D3D_FEATURE_LEVEL d3d_feature_level;
	const D3D_FEATURE_LEVEL feature_level_arr[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, create_device_flags, feature_level_arr, 2, D3D11_SDK_VERSION, &sd, &dxgi_swap_chain, &d3d11_device, &d3d_feature_level, &d3d11_device_context);
	if (res == DXGI_ERROR_UNSUPPORTED)
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, create_device_flags, feature_level_arr, 2, D3D11_SDK_VERSION, &sd, &dxgi_swap_chain, &d3d11_device, &d3d_feature_level, &d3d11_device_context);
	if (res != S_OK)
		return false;

	create_render_target();
	return true;
}

void gui::overlay::cleanup_device_d3d()
{
	cleanup_render_target();

	if (dxgi_swap_chain)
	{
		dxgi_swap_chain->Release();
		dxgi_swap_chain = nullptr;
	}

	if (d3d11_device_context)
	{
		d3d11_device_context->Release();
		d3d11_device_context = nullptr;
	}

	if (d3d11_device)
	{
		d3d11_device->Release();
		d3d11_device = nullptr;
	}
}

void gui::overlay::create_render_target()
{
	ID3D11Texture2D* d3d11_back_buffer;
	dxgi_swap_chain->GetBuffer(0, IID_PPV_ARGS(&d3d11_back_buffer));
	if (d3d11_back_buffer != nullptr)
	{
		d3d11_device->CreateRenderTargetView(d3d11_back_buffer, nullptr, &d3d11_render_target_view);
		d3d11_back_buffer->Release();
	}
}

void gui::overlay::cleanup_render_target()
{
	if (d3d11_render_target_view)
	{
		d3d11_render_target_view->Release();
		d3d11_render_target_view = nullptr;
	}
}

LRESULT __stdcall gui::overlay::window_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (d3d11_device != nullptr && wParam != SIZE_MINIMIZED)
		{
			cleanup_render_target();
			dxgi_swap_chain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			create_render_target();
		}
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}