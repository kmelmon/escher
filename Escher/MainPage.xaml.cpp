//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <ppltasks.h>
#include <memorybuffer.h>

using namespace Escher;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Shapes;
using namespace Windows::Storage::Streams;
using namespace Windows::Graphics::Imaging;
using namespace concurrency;
using namespace Microsoft::WRL;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();
	LoadImages();
	InitializeTileGrid();
	Window::Current->SizeChanged += ref new Windows::UI::Xaml::WindowSizeChangedEventHandler(this, &Escher::MainPage::OnSizeChanged);
}

void Escher::MainPage::OnSizeChanged(Platform::Object^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ e)
{
	InitializeTileGrid();
	UpdateImageBrushes();
}

void MainPage::LoadImages()
{
	wchar_t* tiles[] =
	{
		L"ms-appx:///Assets/tile5.bmp",
		L"ms-appx:///Assets/tile4.bmp",
		L"ms-appx:///Assets/tile3.bmp",
		L"ms-appx:///Assets/tile2.bmp",
		L"ms-appx:///Assets/tile.bmp",
	};
	m_numFrames = ARRAYSIZE(tiles);

	for (unsigned int i = 0; i < ARRAYSIZE(tiles); i++)
	{
		create_task(GenerateTileAsync(tiles[i])).then([this, i](_In_ SoftwareBitmap^ bitmap)
			{
				// For some reason SoftwareBitmapSource requires premultiplied alpha otherwise it throws an exception.  Workaround this limitation by converting.
				SoftwareBitmap^ tileBitmap = SoftwareBitmap::Convert(bitmap, BitmapPixelFormat::Bgra8, BitmapAlphaMode::Premultiplied);
				auto tileSource = ref new SoftwareBitmapSource();
				tileSource->SetBitmapAsync(tileBitmap);
				m_tiles.Append(tileSource);
				SoftwareBitmap^ inverseTileBitmap = InvertTile(tileBitmap);
				auto tileInverseSource = ref new SoftwareBitmapSource();
				tileInverseSource->SetBitmapAsync(inverseTileBitmap);
				m_inverseTiles.Append(tileInverseSource);	

				if (m_tiles.Size == m_numFrames)
				{
					m_timer = ref new Windows::UI::Xaml::DispatcherTimer();
					Windows::Foundation::TimeSpan ts;
					ts.Duration = 3000000;
					m_timer->Interval = ts;
					m_timer->Tick += ref new Windows::Foundation::EventHandler<Platform::Object^>(this, &MainPage::OnTick);
					m_timer->Start();
				}
			});
	}
}

void MainPage::OnTick(Platform::Object ^ sender, Platform::Object ^ args)
{
	UpdateImageBrushes();
	m_frame++;
	if (m_frame == m_numFrames)
	{
		m_frame = 0;
	}
}

IAsyncOperation<SoftwareBitmap^>^ MainPage::GenerateTileAsync(wchar_t* uri)
{
	return create_async([uri]()
	{
		Uri^ tileUri = ref new Uri(ref new Platform::String(uri));
		auto streamReference = RandomAccessStreamReference::CreateFromUri(tileUri);
		return create_task(streamReference->OpenReadAsync()).then([](_In_ IRandomAccessStreamWithContentType^ stream)
		{
			return BitmapDecoder::CreateAsync(stream);
		}, task_continuation_context::use_arbitrary())
		.then([](_In_ BitmapDecoder^ decoder)
		{
			return decoder->GetSoftwareBitmapAsync();
		}, task_continuation_context::use_arbitrary())
		.then([](_In_ task<SoftwareBitmap^> t)
		{
			try
			{
				return t.get();
			}
			catch (Exception^ e)
			{
				OutputDebugString(e->Message->Data());
				return (SoftwareBitmap^)nullptr;
			}
		}, task_continuation_context::use_arbitrary());
	});
}

SoftwareBitmap^ MainPage::InvertTile(SoftwareBitmap^ tile)
{
	auto inverse = SoftwareBitmap::Copy(tile);
	auto bitmapBuffer = inverse->LockBuffer(BitmapBufferAccessMode::ReadWrite);
	BitmapPlaneDescription bitmapPlaneDescription = bitmapBuffer->GetPlaneDescription(0);
	int stride = bitmapPlaneDescription.Stride;
	IMemoryBufferReference^ memoryBufferReference = bitmapBuffer->CreateReference();
	ComPtr<IInspectable> iMemoryBufferReference = reinterpret_cast<IInspectable*>(memoryBufferReference);
	ComPtr<IMemoryBufferByteAccess> memoryBufferByteAccess;
	iMemoryBufferReference.As(&memoryBufferByteAccess);
	BYTE* bitmapBytes;
	unsigned int capacity;
	memoryBufferByteAccess->GetBuffer(&bitmapBytes, &capacity);
	for (unsigned int i = 0; i < capacity; i += 4)
	{
		bitmapBytes[i] = ~bitmapBytes[i];
		bitmapBytes[i + 1] = ~bitmapBytes[i + 1];
		bitmapBytes[i + 2] = ~bitmapBytes[i + 2];
		// 4th channel is alpha, don't invert
	}

	return inverse;
}

int tileSize = 100;

void MainPage::InitializeTileGrid()
{
	tileGrid->Children->Clear();

	auto window = Window::Current;
	int rows = static_cast<int>(window->Bounds.Height) / tileSize + 1;
	int cols = static_cast<int>(window->Bounds.Width) / tileSize + 1;
	tileGrid->RowDefinitions->Clear();
	tileGrid->ColumnDefinitions->Clear();
	for (int i = 0; i < rows; i++)
	{
		auto rowDefinition = ref new RowDefinition();
		rowDefinition->Height = tileSize;
		tileGrid->RowDefinitions->Append(rowDefinition);
	}
	for (int j = 0; j < cols; j++)
	{
		auto columnDefinition = ref new ColumnDefinition();
		columnDefinition->Width = tileSize;
		tileGrid->ColumnDefinitions->Append(columnDefinition);
	}

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			auto rect = ref new Rectangle();
			Grid::SetRow(safe_cast<FrameworkElement^>(rect), i);
			Grid::SetColumn(safe_cast<FrameworkElement^>(rect), j);
			tileGrid->Children->Append(rect);

			auto imageBrush = ref new ImageBrush();

			auto transform = ref new RotateTransform();
			bool evenRow = (i % 2 == 0);
			bool evenCol = (j % 2 == 0);
			int angleMultiplier = evenRow ? 0 : -1;
			int delta = (angleMultiplier == 0) ? 1 : -1;
			int increment = evenCol ? 0 : delta;
			angleMultiplier = angleMultiplier + increment;
			transform->Angle = 90 * angleMultiplier;
			transform->CenterX = 50;
			transform->CenterY = 50;
			imageBrush->Transform = transform;
			rect->Fill = imageBrush;
		}
	}
}

void MainPage::UpdateImageBrushes()
{
	auto window = Window::Current;
	int rows = static_cast<int>(window->Bounds.Height) / tileSize + 1;
	int cols = static_cast<int>(window->Bounds.Width) / tileSize + 1;
	int count = 0;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			auto rect = safe_cast<Rectangle^>(tileGrid->Children->GetAt(count++));
			auto imageBrush = safe_cast<ImageBrush^>(rect->Fill);
			bool invert = ((i % 2) + (j % 2)) % 2;
			imageBrush->ImageSource = invert ? m_inverseTiles.GetAt(m_frame) : m_tiles.GetAt(m_frame);
			
		}
	}
}


