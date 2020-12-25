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
	int dummy = 10;
}

void MainPage::LoadImages()
{
	create_task(GenerateTileAsync()).then([this](_In_ SoftwareBitmap^ bitmap)
		{
			m_tile = bitmap;
			m_tileInverse = InvertTile(m_tile);
			m_tileSource = ref new SoftwareBitmapSource();
			m_tileSource->SetBitmapAsync(m_tile);
		});
}

IAsyncOperation<SoftwareBitmap^>^ MainPage::GenerateTileAsync()
{
	return create_async([]()
	{
		Uri^ tileUri = ref new Uri("ms-appx:///Assets/tile.gif");
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
		bitmapBytes[i + 3] = ~bitmapBytes[i + 3];
	}

	return inverse;
}

void MainPage::InitializeTileGrid()
{
	tileGrid->Children->Clear();

	int tileSize = 100;

	auto window = Window::Current;
	int rows = static_cast<int>(window->Bounds.Height) / tileSize;
	int cols = static_cast<int>(window->Bounds.Width) / tileSize;
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
			bool odd = (i * cols + j) % 2;
			bool invert = ((i % 2) + (j % 2)) % 2;
			auto imageBrush = ref new ImageBrush();
			/*
			auto bitmapImage = ref new BitmapImage();
			bitmapImage->UriSource = invert ? ref new Uri(L"ms-appx:///Assets/face.png") : ref new Uri(L"ms-appx:///Assets/face2.png");
			imageBrush->ImageSource = bitmapImage;
			*/
//			auto inverseTileSource = ref new SoftwareBitmapSource();
//			inverseTileSource->SetBitmapAsync(m_tileInverse);
//			imageBrush->ImageSource = invert ? inverseTileSource : tileSource;
			imageBrush->ImageSource = m_tileSource;
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
//			rect->Fill = ref new SolidColorBrush(Windows::UI::Colors::White);
			Grid::SetRow(safe_cast<FrameworkElement^>(rect), i);
			Grid::SetColumn(safe_cast<FrameworkElement^>(rect), j);
			tileGrid->Children->Append(rect);
		}
	}

}

