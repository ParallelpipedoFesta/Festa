#pragma once

#include "common.h"


namespace Festa {
	namespace CV2 {
		enum VideoCaptureAPIs {
			CAP_ANY = 0,            //!< Auto detect == 0
			CAP_VFW = 200,          //!< Video For Windows (obsolete, removed)
			CAP_V4L = 200,          //!< V4L/V4L2 capturing support
			CAP_V4L2 = CAP_V4L,      //!< Same as CAP_V4L
			CAP_FIREWIRE = 300,          //!< IEEE 1394 drivers
			CAP_FIREWARE = CAP_FIREWIRE, //!< Same value as CAP_FIREWIRE
			CAP_IEEE1394 = CAP_FIREWIRE, //!< Same value as CAP_FIREWIRE
			CAP_DC1394 = CAP_FIREWIRE, //!< Same value as CAP_FIREWIRE
			CAP_CMU1394 = CAP_FIREWIRE, //!< Same value as CAP_FIREWIRE
			CAP_QT = 500,          //!< QuickTime (obsolete, removed)
			CAP_UNICAP = 600,          //!< Unicap drivers (obsolete, removed)
			CAP_DSHOW = 700,          //!< DirectShow (via videoInput)
			CAP_PVAPI = 800,          //!< PvAPI, Prosilica GigE SDK
			CAP_OPENNI = 900,          //!< OpenNI (for Kinect)
			CAP_OPENNI_ASUS = 910,          //!< OpenNI (for Asus Xtion)
			CAP_ANDROID = 1000,         //!< Android - not used
			CAP_XIAPI = 1100,         //!< XIMEA Camera API
			CAP_AVFOUNDATION = 1200,         //!< AVFoundation framework for iOS (OS X Lion will have the same API)
			CAP_GIGANETIX = 1300,         //!< Smartek Giganetix GigEVisionSDK
			CAP_MSMF = 1400,         //!< Microsoft Media Foundation (via videoInput)
			CAP_WINRT = 1410,         //!< Microsoft Windows Runtime using Media Foundation
			CAP_INTELPERC = 1500,         //!< RealSense (former Intel Perceptual Computing SDK)
			CAP_REALSENSE = 1500,         //!< Synonym for CAP_INTELPERC
			CAP_OPENNI2 = 1600,         //!< OpenNI2 (for Kinect)
			CAP_OPENNI2_ASUS = 1610,         //!< OpenNI2 (for Asus Xtion and Occipital Structure sensors)
			CAP_GPHOTO2 = 1700,         //!< gPhoto2 connection
			CAP_GSTREAMER = 1800,         //!< GStreamer
			CAP_FFMPEG = 1900,         //!< Open and record video file or stream using the FFMPEG library
			CAP_IMAGES = 2000,         //!< OpenCV Image Sequence (e.g. img_%02d.jpg)
			CAP_ARAVIS = 2100,         //!< Aravis SDK
			CAP_OPENCV_MJPEG = 2200,         //!< Built-in OpenCV MotionJPEG codec
			CAP_INTEL_MFX = 2300,         //!< Intel MediaSDK
			CAP_XINE = 2400,         //!< XINE engine (Linux)
		};
		enum ImageCode: int {
			FLIP_BOTH = -1,
			FLIP_VERTICALLY = 0,
			FLIP_HORIZONTALLY = 1
		};
	}
	struct Color {
		uchar r = 0;
		uchar g = 0;
		uchar b = 0;
		uchar a = 255;
		uchar c = 0;
		Color() {}
		Color(uchar _r) :c(1), r(_r), g(_r), b(_r) {}
		Color(int _r) :c(1), r(_r), g(_r), b(_r) {}
		Color(float _r) :c(1), r(uchar(255.0f * _r)), g(uchar(255.0f * _r)), b(uchar(255.0f * _r)) {}
		Color(uchar _r, uchar _g, uchar _b) :c(3), r(_r), g(_g), b(_b) {}
		Color(int _r, int _g, int _b) :c(3), r(uchar(_r)), g(uchar(_g)), b(uchar(_b)) {}
		Color(float _r, float _g, float _b) :c(3), r(uchar(255.0f * _r)), g(uchar(255.0f * _g)), b(uchar(255.0f * _b)) {}
		Color(const vec3& v) :c(3), r(uchar(255.0f * v.x)), g(uchar(255.0f * v.y)), b(uchar(255.0f * v.z)) {}
		Color(uchar _r, uchar _g, uchar _b, uchar _a) :c(4), r(_r), g(_g), b(_b), a(_a) {}
		Color(int _r, int _g, int _b, int _a) :c(4), r(uchar(_r)), g(uchar(_g)), b(uchar(_b)), a(uchar(_a)) {}
		Color(float _r, float _g, float _b, float _a) :c(4), r(uchar(255.0f * _r)), g(uchar(255.0f * _g)), b(uchar(255.0f * _b)), a(uchar(255.0f * _a)) {}
		Color(const vec4& v) :c(4), r(uchar(255.0f * v.x)), g(uchar(255.0f * v.y)), b(uchar(255.0f * v.z)), a(uchar(255.0f * v.w)) {}
		Color(uint hex) :c(4), r(uchar(hex >> 24)), g(uchar((hex >> 16) & 0xff)), b(uchar((hex >> 8) & 0xff)), a(uchar(hex & 0xff)) {}
		Color(COLORREF color) : c(3), r(uchar(color&0xff)), g(uchar((color >> 8) & 0xff)), b(uchar((color>>16) & 0xff)) {}
		Color(const std::string& code) {
			int color = std::stoi(code, nullptr, 16);
			if (code.size() > 6) {
				c = 4;
				a= uchar(color & 0xff);
				b = uchar((color >> 8) & 0xff);
				g = uchar((color >> 16) & 0xff);
				r = uchar((color >> 24) & 0xff);
			}
			else {
				c = 3;
				b = uchar(color & 0xff);
				g = uchar((color >> 8) & 0xff);
				r = uchar((color >> 16) & 0xff);
			}
		}
		int channels()const {
			return c;
		}
		COLORREF MFC()const {
			return RGB(r, g, b);
		}
		uchar avg()const {
			return uchar((uint(a) + uint(b) + uint(c)) / 3);
		}
		vec3 toVec3()const {
			const vec3 col(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f);
			return channels() == 4 ? col * (float(a) / 255.0f) : col;
		}
		vec4 toVec4()const {
			return vec4(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f, float(a) / 255.0f);
		}
		uint hex()const {
			return uint(r) << 24 | uint(g) << 16 | uint(b) << 8 | uint(a);
		}
		operator vec3()const {
			return toVec3();
		}
		operator vec4()const {
			return toVec4();
		}
		uchar& operator[](char i) {
			return ((uchar*)this)[i];
		}
		const uchar& operator[](char i)const {
			return ((const uchar*)this)[i];
		}
		float grayValue(const vec3& w = vec3(0.2126f, 0.7152f, 0.0722f))const {
			return glm::dot(w, toVec3());
		}
		static Color mix(const Color& col0, const Color& col1, float t) {
			Color ret; ret.c = std::min(col0.c, col1.c);
			for (char i = 0; i < ret.c; i++)
				ret[i] = (uchar)lerpT(float(col0[i]), float(col1[i]), t);
			return ret;
		}
		static Color blend(const Color& bg, const Color& fg) {
			if (bg.channels() < 4 || fg.channels() < 4)return fg;
			return mix(bg, fg, float(fg.a)/255.0f);
		}
	};

	class FBO {
	public:
		FBO() {}
		FBO(int _width, int _height) {
			Generate(_width, _height);
		}
		void Release() {
			if (id) {
				glDeleteFramebuffers(1, &id);
				id = 0;
			}
		}
		~FBO() {
			Release();
		}
		uint ID()const {
			return id;
		}
		int width()const {
			return mWidth;
		}
		int height()const {
			return mHeight;
		}
		void Generate() {
			Release();
			glGenFramebuffers(1, &id);
			bind();
		}
		void Generate(int width, int height) {
			mWidth = width, mHeight = height;
			Generate();

			Init();

			check();
			unbind();
		}
		void _generate(int width, int height) {
			mWidth = width, mHeight = height;
			Generate();
			Init();
			unbind();
		}
		void Resize(int width, int height) {
			if(width!=mWidth||height!=mHeight)Generate(width, height);
		}
		void Resize(const ivec2& size) {
			Resize(size.x, size.y);
		}
		virtual void Init() {

		}
		void bind()const {
			glBindFramebuffer(GL_FRAMEBUFFER, id);
		}
		void Begin()const {
			bind();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, mWidth, mHeight);
		}
		void End()const {
			unbind();
		}
		void RBO(uint& rbo, int storage, int attachment) {
			bind();
			glGenRenderbuffers(1, &rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, storage, width(), height());
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rbo);
			unbind();
		}
		void RBODepth(uint& rbo) {
			RBO(rbo, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT);
		}
		void bindAttachments(const std::vector<uint>& attachments) {
			bind();
			glDrawBuffers((uint)attachments.size(), &attachments[0]);
			unbind();
		}
		static void unbind() {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		static void bind(uint id) {
			glBindFramebuffer(GL_FRAMEBUFFER, id);
		}
		static void check() {
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				LOGGER.error("Incomplete FBO");
		}
	protected:
		int mWidth = 0;
		int mHeight = 0;
		uint id = 0;
	};

	class FrameDepthBuffer: public FBO {
	public:
		FrameDepthBuffer() {}
		FrameDepthBuffer(int _width, int _height) {
			Generate(_width, _height);
		}
		~FrameDepthBuffer() {
			Release();
		}
		void Init() {
			//RBODepth(rbo);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}
	private:
		uint rbo = 0;
	};

	class FrameRenderBuffer: public FBO {
	public:
		FrameRenderBuffer() {}
		FrameRenderBuffer(int _width, int _height) {
			Generate(_width, _height);
		}
		void Release() {
			if (id)glDeleteFramebuffers(1, &id);
			if (rbo)glDeleteRenderbuffers(1, &rbo);
		}
		~FrameRenderBuffer() {
			Release();
		}
		void Init() {
			RBO(rbo, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT);
		}
	private:
		uint rbo = 0;
	};

	class Image {
	public:
		void* _mat=0;
		Image();
		Image(const Path& file) {
			Load(file);
		}
		void grab(int x, int y, int w, int h);
		Image(void* data, int w, int h, int c) {
			Init(data, w, h, c);
		}
		Image(const Color& color, int w, int h) {
			Init(color, w, h);
		}
		void Load(const Path& file);
		void Init(void* data, int w, int h, int c);
		void Init(const Color& color, int w, int h);
		void decodeBase64(const std::string& base64);
		std::string encodeBase64(const std::string& ext)const;

		Image(const Image& x);
		void operator=(const Image& x);
		void release();
		~Image();
		void show(const std::string& title, int delay = 0)const;
		void save(const std::string& path)const;
		int width()const;
		int height()const;
		int channels()const;
		int size()const {
			return channels() * width() * height();
		}
		uchar* data()const;
		uchar& get(int x,int y,int c=0) {
			return data()[ull(channels())*(ull(y)*ull(width())+ull(x))+ull(c)];
		}
		const uchar& get(int x, int y, int c = 0)const {
			return data()[ull(channels()) * (ull(y) * ull(width()) + ull(x)) + ull(c)];
		}
		Color GetColor(int x, int y)const {
			switch (channels()) {
			case 1:
				return Color(get(x, y));
			case 3:
				return Color(get(x, y, 2), get(x, y, 1), get(x, y, 0));
			case 4:
				return Color(get(x, y, 2), get(x, y, 1), get(x, y, 0), get(x, y, 3));
			}
			return Color();
		}
		void SetColor(int x, int y, const Color& col) {
			switch (channels()) {
			case 1:
				get(x, y) = col.r;
				break;
			case 3:
				get(x, y, 0) = col.b;
				get(x, y, 1) = col.g;
				get(x, y, 2) = col.r;
				break;
			case 4:
				get(x, y, 0) = col.b;
				get(x, y, 1) = col.g;
				get(x, y, 2) = col.r;
				get(x, y, 3) = col.a;
				break;
			}
		}
		bool empty()const;
		void resetChannels(int value);
		void bgr2rgb();
		void putImage(Image* img, int x, int y) {
			assert(channels() == img->channels());
			for (int i = 0; i < img->width(); i++) {
				int  u = x + i;
				if (u < 0)continue;
				else if (u >= width())break;
				for (int j = 0; j < img->height(); j++) {
					int v = y + j;
					if (v < 0)continue;
					else if (v >= height())break;
					for (int c = 0; c < channels(); c++)
						get(u, v, c) = img->get(i,j,c);
				}
			}
		}
		void blendImage(Image* img, int x, int y) {
			assert(channels() == img->channels());
			for (int i = 0; i < img->width(); i++) {
				int  u = x + i;
				if (u < 0)continue;
				else if (u >= width())break;
				for (int j = 0; j < img->height(); j++) {
					int v = y + j;
					if (v < 0)continue;
					else if (v >= height())break;
					Color col = Color::blend(GetColor(u, v), img->GetColor(i, j));
					//col.a = 255;
					SetColor(u, v, col);
				}
			}
		}
		void Circle(int x, int y, int r, const Color& color, int thickness=-1);
		void Rectangle(int x1, int y1, int x2, int y2, const Color& color, int thickness=-1);
		void Resize(float fx, float fy);
		void Resize(Image* img, float fx, float fy)const;
		void Resize(int w, int h);
		void Resize(Image* img, int w, int h)const;
		void Flip(int flipCode);
		operator bool()const;

		int checkImpl()const;
		int checkInited()const;
	private:
		bool inMap = false;
	};

	template<uint Target>
	class GLTexture {
	public:
		GLTexture() {}
		GLTexture(uint _id) :id(_id) {}

		uint ID()const {
			return id;
		}
		void Release() {
			if (id) {
				glDeleteTextures(1, &id);
				id = 0;
			}
		}
		~GLTexture() {
			Release();
		}
		void bind()const {
			glBindTexture(Target, id);
		}
		void bind(uint texture_id)const {
			glActiveTexture(GL_TEXTURE0 + texture_id);
			bind();
		}
		void create() {
			Release();
			glGenTextures(1, &id);
		}
		bool empty()const {
			return !id;
		}
		operator bool()const {
			return id;
		}
		static void unbind() {
			glBindTexture(Target, 0);
		}
		static void wrapping(uint name, int param) {
			glTexParameteri(Target, name, param);
		}
		static void wrapping1D(int param) {
			glTexParameteri(Target, GL_TEXTURE_WRAP_S, param);
		}
		static void wrapping2D(int param) {
			glTexParameteri(Target, GL_TEXTURE_WRAP_S, param);
			glTexParameteri(Target, GL_TEXTURE_WRAP_T, param);
		}
		static void wrapping3D(int param) {
			glTexParameteri(Target, GL_TEXTURE_WRAP_S, param);
			glTexParameteri(Target, GL_TEXTURE_WRAP_T, param);
			glTexParameteri(Target, GL_TEXTURE_WRAP_R, param);
		}
		static void borderColor(const Color& color = Color(0, 0, 0)) {
			vec4 border = color;
			glTexParameterfv(Target, GL_TEXTURE_BORDER_COLOR, &border.x);
		}
		static void minFilter(int param) {
			glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, param);
		}
		static void magFilter(int param) {
			glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, param);
		}
	protected:
		uint id = 0;
	};

	class Texture1D: public GLTexture<GL_TEXTURE_1D> {
	public:
		Texture1D() { id = 0; }
		Texture1D(const void* data, uint length, uint format, uint type) {
			generate(data, length, format, type);
		}
		/*void randomize(size_t size, int channels, float low = 0.0f, float high = 1.0f) {
			uint format
			switch (format) {
			case GL_RED:
				channels = 1;
				break;
			case GL_RGB:
				c
			}

			size *= channels;
			std::vector<float> data(size);
			for (uint i = 0; i < size; i++)data[i] = randT(low, high);
			init(&data[0], size, format);
		}*/
		void generate(const void* data, uint length, int format, uint type = GL_FLOAT) {
			glGenTextures(1, &id);
			bind();
			glTexImage1D(GL_TEXTURE_1D, 0, format, length, 0, format, type, data);
			glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

		}
	private:
		
	};

	class Texture : public GLTexture<GL_TEXTURE_2D> {
	public:
		Texture() {}
		Texture(uint _id) {
			id = _id;
		}
		Texture(const Image& img) {
			//if (img.checkImpl())throw "Failed to create texture2d object";
			Release();
			uint from = 0, to = 0;
			switch (img.channels()) {
			case 1:
				from = GL_RED, to = GL_RED; break;
			case 3:
				from = GL_BGR, to = GL_RGB; break;
			case 4:
				from = GL_BGRA, to = GL_RGBA; break;
			}
			Generate(img.width(), img.height(), img.data(), from, to);
			setParameters();
		}
		void Generate(const Image& img) {
			//if (img.checkImpl())throw "Failed to create texture2d object";
			Release();
			uint from = 0, to = 0;
			switch (img.channels()) {
			case 1:
				from = GL_RED, to = GL_RED; break;
			case 3:
				from = GL_BGR, to = GL_RGB; break;
			case 4:
				from = GL_BGRA, to = GL_RGBA; break;
			}
			Generate(img.width(), img.height(), img.data(), from, to);
			setParameters();
		}
		void FromFBO(const FBO& fbo, int attachment, int internalFormat, int format, int type = GL_UNSIGNED_BYTE, int filter = GL_LINEAR) {
			Generate(fbo.width(), fbo.height(), 0, internalFormat, format, type);
			minFilter(filter);
			magFilter(filter);
			bindToFBO(fbo, attachment);
		}
		void FromRenderBuffer(const FrameRenderBuffer& fbo, int internalFormat = GL_RGB, int format = GL_RGB, int type = GL_UNSIGNED_BYTE, int filter = GL_LINEAR) {
			FromFBO(fbo, GL_COLOR_ATTACHMENT0, format, format, type, filter);
		}
		void FromDepthBuffer(const FrameDepthBuffer& fbo, int type = GL_UNSIGNED_BYTE, int filter = GL_LINEAR) {
			FromFBO(fbo, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, type, filter);
		}
		~Texture() {
			Release();
		}
		
		void bindToFBO(const FBO& fbo, int attachment) {
			fbo.bind();
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, id, 0);
			FBO::unbind();
		}
		void Generate(int width, int height, const void* pixels, uint internalFormat, uint format, uint type = GL_UNSIGNED_BYTE) {
			create();
			bind();
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, internalFormat, type, pixels);
		}
		void Transfer(Texture& texture) {
			texture.id = id;
			id = 0;
		}
		static void borderColor(const Color& color = Color(0, 0, 0)) {
			vec4 border = color;
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &border.x);
		}
		static void minFilter(int param = GL_NEAREST) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param);
		}
		static void magFilter(int param = GL_LINEAR) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
		}
		static void setParameters(int wrapParam = GL_REPEAT, int minFilterParam = GL_LINEAR_MIPMAP_LINEAR,
			int magFilterParam = GL_LINEAR, const Color& border = Color(0, 0, 0)) {
			wrapping2D(wrapParam);
			borderColor(border);
			minFilter(minFilterParam);
			magFilter(magFilterParam);
			if ((GL_NEAREST_MIPMAP_NEAREST <= minFilterParam && minFilterParam <= GL_LINEAR_MIPMAP_LINEAR) ||
				(GL_NEAREST_MIPMAP_NEAREST <= magFilterParam && magFilterParam <= GL_LINEAR_MIPMAP_LINEAR))
				glGenerateMipmap(GL_TEXTURE_2D);
		}
	};

	struct GBuffer :public FBO {
		Texture gPosition;
		Texture gNormal;
		Texture gAlbedoSpec;
		uint rboDepth = 0;
		GBuffer() {

		}
		~GBuffer() {
			Release();
		}
		void Init() {
			gPosition.FromFBO(*this, GL_COLOR_ATTACHMENT0, GL_RGBA, GL_RGBA16F, GL_FLOAT, GL_NEAREST);
			gNormal.FromFBO(*this, GL_COLOR_ATTACHMENT1, GL_RGBA, GL_RGBA16F, GL_FLOAT, GL_NEAREST);
			gAlbedoSpec.FromFBO(*this, GL_COLOR_ATTACHMENT2, GL_RGBA, GL_RGBA16F, GL_FLOAT, GL_NEAREST);
			bindAttachments({ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 });
			RBODepth(rboDepth);
		}
	};

	class TextureMap {
	public:
		TextureMap() {}
		TextureMap(const ivec2& stride, const ivec2& count, int channels = 3) {
			Init(stride, count, channels);
		}
		void Init(const ivec2& stride, const ivec2& count, int channels = 3) {
			mSize = count;
			mStride = stride;
			Color col;
			switch (channels) {
			case 1:
				col = Color(0);
				break;
			case 3:
				col = Color(0, 0, 0);
				break;
			case 4:
				col = Color(0, 0, 0, 0);
				break;
			}
			mImage.Init(col, mStride.x * mSize.x, mStride.y * mSize.y);
		}
		Texture& GetTexture() {
			return mTexture;
		}
		const Texture& GetTexture()const {
			return mTexture;
		}
		void GenerateTexture() {
			mTexture.Generate(mImage);
		}
		ivec2 stride()const {
			return mStride;
		}
		vec2 textureCoordStride()const {
			return 1.0f / vec2(mSize);
		}
		vec2 mapSize()const {
			return vec2(float(mapWidth()), float(mapHeight()));
		}
		int capacity()const {
			return mSize.x * mSize.y;
		}
		int numStored()const {
			return int(mNumStored);
		}
		int mapWidth()const {
			return mImage.width();
		}
		int mapHeight()const {
			return mImage.height();
		}
		bool Find(const Path& img)const {
			return mMapping.find(img) != mMapping.end();
		}
		vec2 GetTextureCoords(const Path& img) {
			return mMapping[img];
		}
		vec2 Insert(const Path& img) {
			if (Find(img))return GetTextureCoords(img);
			Image im(img);
			vec2 ret = Store(im);
			return mMapping[img] = ret;
		}
		vec2 Next(const std::string& name) {
			ivec2 pos = GetNewPosition();
			return mMapping[name] = vec2(pos) / mapSize();
		}
		ivec2 GetNewID()const {
			int id = numStored();
			return ivec2(id % mSize.x, id / mSize.x);
		}
		ivec2 GetNewPosition()const {
			return GetNewID() * mStride;
		}
		vec2 Store(Image& img) {
			if (numStored() == capacity())return vec2(-1.0f);
			ivec2 pos = GetNewPosition();
			img.Resize(mStride.x, mStride.y);
			for (int y = 0; y < mStride.y; y++) {
				int py = pos.y + y;
				if (py >= mImage.height())break;
				for (int x = 0; x < mStride.x; x++) {
					int px = pos.x + x;
					if (px >= mImage.width())break;
					mImage.SetColor(px, py, img.GetColor(x, y));
				}
			}
			mNumStored++;
			return vec2(pos) / mapSize();
		}
		//
		void Debug() {
			mImage.show("img");
		}
	private:
		std::map<std::string, vec2> mMapping;
		Texture mTexture;
		Image mImage;
		ivec2 mSize = ivec2(0);
		ivec2 mStride = vec2(0);
		int mNumStored = 0;
	};

	class Video {
	public:
		Video() {}
		Video(const Path& file) {
			init(file);
		}
		Video(int device) {
			init(device);
		}
		~Video() {
			release();
		}
		void load(const Path& file) {
			init(file);
		}
		void init(const Path& file);
		void init(int device);
		VirtualValue<int> width()const;
		VirtualValue<int> height()const;
		VirtualValue<double> fps()const;
		VirtualValue<int> posFrames()const;
		VirtualValue<double> time()const;
		VirtualValue<int> frameCount()const;
		double duration()const {
			return frameCount()/fps();
		}
		bool isFinished()const;
		bool empty()const;
		void release();
		void operator>>(Image& img)const;
		void operator>>(Texture& texture)const;
		void show(const std::string& title);
		operator bool()const {
			return _cap;
		}
	private:
		void* _cap = 0;
	};

	class VideoWriter {
	public:
		VideoWriter() {}
		VideoWriter(const std::string& file, int fps, int width, int height, int fourcc = CV2::CAP_OPENCV_MJPEG) {
			open(file, fps, width, height, fourcc);
		}
		void open(const std::string& file, int fps, int width, int height, int fourcc = CV2::CAP_OPENCV_MJPEG);
		void release();
		~VideoWriter() {
			release();
		}
		void write(const Image& image);
		void write(const Video& video);
	private:
		void* _writer = 0;
	};
}
