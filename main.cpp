/*
    Copyright (C) 2012 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Author: Volker Krause <volker.krause@kdab.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

struct enum_t {
    EGLint value;
    const char* displayName;
};

static enum_t boolMap[] {
    { EGL_TRUE, "true" },
    { EGL_FALSE, "false" }
};

static enum_t bufferTypeMap[] {
    { EGL_RGB_BUFFER, "RGB" },
    { EGL_LUMINANCE_BUFFER, "Luminance" }
};

static enum_t caveatMap[] {
    { EGL_NONE, "none" },
    { EGL_SLOW_CONFIG, "slow" },
    { EGL_NON_CONFORMANT_CONFIG, "non-conformant" }
};

static enum_t transparentTypeMap[] {
    { EGL_NONE, "none" },
    { EGL_TRANSPARENT_RGB, "transparent RGB" }
};

static enum_t surfaceTypeMap[] {
    { EGL_PBUFFER_BIT, "pbuffer" },
    { EGL_PIXMAP_BIT, "pixmap" },
    { EGL_WINDOW_BIT, "window" },
    { EGL_VG_COLORSPACE_LINEAR_BIT, "VG (linear colorspace)" },
    { EGL_VG_ALPHA_FORMAT_PRE_BIT, "VG (alpha format pre)" },
    { EGL_MULTISAMPLE_RESOLVE_BOX_BIT, "multisample resolve box" },
    { EGL_SWAP_BEHAVIOR_PRESERVED_BIT, "swap behavior preserved" },
#ifdef EGL_STREAM_BIT_KHR
    { EGL_STREAM_BIT_KHR, "stream" },
#endif
};

static enum_t renderableTypeMap[] {
    { EGL_OPENGL_ES_BIT, "OpenGL ES" },
    { EGL_OPENVG_BIT, "OpenVG" },
    { EGL_OPENGL_ES2_BIT, "OpenGL ES2" },
    { EGL_OPENGL_BIT, "OpenGL" },
#ifdef EGL_OPENGL_ES3_BIT
    { EGL_OPENGL_ES3_BIT, "OpenGL ES3" }
#endif
};


struct attrib_t {
    EGLint attribute;
    const char* displayName;
    enum_t* enumMap;
    int enumMapSize;
    bool isFlag;
};

#define A_NUM(x) { x, #x, 0, 0, false }
#define A_MAP(x, map) { x, #x, map, sizeof(map) / sizeof(enum_t), false }
#define A_FLAG(x, map) { x, #x, map, sizeof(map) / sizeof(enum_t), true }

static attrib_t attributes[] {
    A_NUM(EGL_ALPHA_SIZE),
    A_NUM(EGL_ALPHA_MASK_SIZE),
    A_MAP(EGL_BIND_TO_TEXTURE_RGB, boolMap),
    A_MAP(EGL_BIND_TO_TEXTURE_RGBA, boolMap),
    A_NUM(EGL_BLUE_SIZE),
    A_NUM(EGL_BUFFER_SIZE),
    A_MAP(EGL_COLOR_BUFFER_TYPE, bufferTypeMap),
    A_MAP(EGL_CONFIG_CAVEAT, caveatMap),
    A_NUM(EGL_CONFIG_ID),
    A_FLAG(EGL_CONFORMANT, renderableTypeMap),
    A_NUM(EGL_DEPTH_SIZE),
    A_NUM(EGL_GREEN_SIZE),
    A_NUM(EGL_LEVEL),
    A_NUM(EGL_LUMINANCE_SIZE),
    A_NUM(EGL_MAX_PBUFFER_WIDTH),
    A_NUM(EGL_MAX_PBUFFER_HEIGHT),
    A_NUM(EGL_MAX_PBUFFER_PIXELS),
    A_NUM(EGL_MAX_SWAP_INTERVAL),
    A_NUM(EGL_MIN_SWAP_INTERVAL),
    A_MAP(EGL_NATIVE_RENDERABLE, boolMap),
    A_NUM(EGL_NATIVE_VISUAL_ID),
    A_NUM(EGL_NATIVE_VISUAL_TYPE),
    A_NUM(EGL_RED_SIZE),
    A_FLAG(EGL_RENDERABLE_TYPE, renderableTypeMap),
    A_NUM(EGL_SAMPLE_BUFFERS),
    A_NUM(EGL_SAMPLES),
    A_NUM(EGL_STENCIL_SIZE),
    A_FLAG(EGL_SURFACE_TYPE, surfaceTypeMap),
    A_MAP(EGL_TRANSPARENT_TYPE, transparentTypeMap),
    A_NUM(EGL_TRANSPARENT_RED_VALUE),
    A_NUM(EGL_TRANSPARENT_GREEN_VALUE),
    A_NUM(EGL_TRANSPARENT_BLUE_VALUE)
};

#undef A_NUM
#undef A_MAP
#undef A_FLAG

static const int attributesSize = sizeof(attributes) / sizeof(attrib_t);

struct device_property_t {
    EGLint name;
    const char* displayName;
    const char* extension;
    enum Type {
        String,
        Attribute
    } type;
};

static const device_property_t deviceProperties[] {
    { EGL_DRM_DEVICE_FILE_EXT, "DRM device file", "EGL_EXT_device_drm", device_property_t::String },
    { EGL_CUDA_DEVICE_NV, "CUDA device", "EGL_NV_device_cuda", device_property_t::Attribute }
};

static const int devicePropertiesSize = sizeof(deviceProperties) / sizeof(device_property_t);


static void printEnum(int value, attrib_t *attr)
{
    for (int i = 0; i < attr->enumMapSize; ++i) {
        enum_t *enumValue = &attr->enumMap[i];
        if (value == enumValue->value) {
            cout << enumValue->displayName;
            return;
        }
    }
    cout << "0x" << hex << value << dec;
}

static void printFlags(int value, attrib_t *attr)
{
    bool firstEntry = true;
    int handledFlags = 0;
    for (int i = 0; i < attr->enumMapSize; ++i) {
        enum_t *enumValue = &attr->enumMap[i];
        if (value & enumValue->value) {
            if (!firstEntry)
                cout << ", ";
            cout << enumValue->displayName;
            firstEntry = false;
            handledFlags |= enumValue->value;
        }
    }

    if (handledFlags != value) {
        if (!firstEntry)
            cout << ", ";
        cout << "unhandled flags 0x" << hex << (value - handledFlags) << dec;
    }
}

static void printOutputLayers(EGLDisplay display, const char* indent = "")
{
#ifdef EGL_EXT_output_base
    const auto eglGetOutputLayersEXT = reinterpret_cast<PFNEGLGETOUTPUTLAYERSEXTPROC>(eglGetProcAddress("eglGetOutputLayersEXT"));

    EGLint num_layers = 0;
    if (!eglGetOutputLayersEXT(display, nullptr, nullptr, 0, &num_layers)) {
        cout << indent << "Failed to query output layers." << endl;
        return;
    }
    cout << indent << "Found " << num_layers << " output layers." << endl;
#endif
}

static void printOutputPorts(EGLDisplay display, const char* indent = "")
{
#ifdef EGL_EXT_output_base
    const auto eglGetOutputPortsEXT = reinterpret_cast<PFNEGLGETOUTPUTPORTSEXTPROC>(eglGetProcAddress("eglGetOutputPortsEXT"));

    EGLint num_ports = 0;
    if (!eglGetOutputPortsEXT(display, nullptr, nullptr, 0, &num_ports)) {
        cout << indent << "Failed to query output ports." << endl;
        return;
    }
    cout << indent << "Found " << num_ports << " output ports." << endl;
#endif
}

static void printDisplay(EGLDisplay display, const char* indent = "")
{
    EGLint majorVersion, minorVersion;
    if (!eglInitialize(display, &majorVersion, &minorVersion)) {
        cerr << "Could not initialize EGL!" << endl;
        exit(1);
    }

    cout << indent << "EGL version: " << majorVersion << "." << minorVersion << endl;
    const char* clientAPIs = eglQueryString(display, EGL_CLIENT_APIS);
    cout << indent << "Client APIs for display: " << clientAPIs << endl;
    const char* vendor = eglQueryString(display, EGL_VENDOR);
    cout << indent << "Vendor: " << vendor << endl;
    const char* displayExts = eglQueryString(display, EGL_EXTENSIONS);
    cout << indent << "Display extensions: " << displayExts << endl;

    printOutputLayers(display, indent);
    printOutputPorts(display, indent);

    EGLint numConfigs;
    if (!eglGetConfigs(display, 0, 0, &numConfigs) && numConfigs > 0) {
        cerr << "Could not retrieve the number of EGL configurations!" << endl;
        exit(1);
    }

    cout << indent << "Found " << numConfigs << " configurations." << endl;

    EGLConfig *configs = new EGLConfig[numConfigs];
    if (!eglGetConfigs(display, configs, numConfigs, &numConfigs)) {
        cerr << "Could not retrieve EGL configurations!" << endl;
        exit(1);
    }

    for (int i = 0; i < numConfigs; ++i) {
        cout << indent << "Configuration " << i << ":" << endl;
        for (int j = 0; j < attributesSize; ++j) {
            attrib_t *attr = &attributes[j];
            EGLint value;
            EGLBoolean result = eglGetConfigAttrib(display, configs[i], attr->attribute, &value);
            cout << indent << "  " << attr->displayName << ": ";
            if (result) {
                if (attr->enumMap) {
                    if (!attr->isFlag)
                        printEnum(value, attr);
                    else
                        printFlags(value, attr);
                } else {
                    cout << value;
                }
            } else {
                cout << "<failed>";
            }
            cout << endl;
        }
        cout << endl;
    }

    delete[] configs;
}

#ifdef EGL_EXT_device_base

static EGLDisplay displayForDevice(EGLDeviceEXT device)
{
#ifdef EGL_EXT_platform_base
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayExt = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
    EGLint attribs[] = { EGL_NONE };
    EGLDisplay display = eglGetPlatformDisplayExt(EGL_PLATFORM_DEVICE_EXT, device, attribs);
    return display;
#else
#warning "Compiling without EGL_EXT_platform_base extension support!"
    return EGL_NO_DISPLAY;
#endif
}

static void printDevices()
{
    PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT = reinterpret_cast<PFNEGLQUERYDEVICESEXTPROC>(eglGetProcAddress("eglQueryDevicesEXT"));
    EGLDeviceEXT devices[32];
    EGLint num_devices;
    if (!eglQueryDevicesEXT(32, devices, &num_devices)) {
        cout << "Failed to query devices." << endl << endl;
        return;
    }
    if (num_devices == 0) {
        cout << "Found no devices." << endl << endl;
        return;
    }

    cout << "Found " << num_devices << " device(s)." << endl;
    PFNEGLQUERYDEVICEATTRIBEXTPROC eglQueryDeviceAttribEXT = reinterpret_cast<PFNEGLQUERYDEVICEATTRIBEXTPROC>(eglGetProcAddress("eglQueryDeviceAttribEXT"));
    PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT = reinterpret_cast<PFNEGLQUERYDEVICESTRINGEXTPROC>(eglGetProcAddress("eglQueryDeviceStringEXT"));

    for (int i = 0; i < num_devices; ++i) {
        cout << "Device " << i << ":" << endl;
        EGLDeviceEXT device = devices[i];
        const char* devExts = eglQueryDeviceStringEXT(device, EGL_EXTENSIONS);
        if (devExts) {
            cout << "  Device Extensions: ";
            if (strlen(devExts))
                cout << devExts << endl;
            else
                cout << "none" << endl;
        } else {
            cout << "  Failed to retrieve device extensions." << endl;
        }

        for (int j = 0; j < devicePropertiesSize; ++j) {
            const auto property = deviceProperties[j];
            if (!devExts || strstr(devExts, property.extension) == nullptr)
                continue;
            switch (property.type) {
                case device_property_t::String:
                {
                    const char* value = eglQueryDeviceStringEXT(device, property.name);
                    cout << "  " << property.displayName << ": " << value << endl;
                    break;
                }
                case device_property_t::Attribute:
                {
                    EGLAttrib attrib;
                    if (eglQueryDeviceAttribEXT(device, property.name, &attrib) == EGL_FALSE)
                        break;
                    cout << "  " << property.displayName << ": " << attrib << endl;
                    break;
                }
            }
        }

        EGLDisplay display = displayForDevice(device);
        if (display == EGL_NO_DISPLAY) {
            cout << "  No attached display." << endl;
        }else {
            cout << "  Device display:" << endl;
            printDisplay(display, "    ");
        }

        cout << endl;
    }
}
#endif

int main(int argc, char** argv)
{
    const char* clientExts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (clientExts)
        cout << "Client extensions: " << clientExts << endl << endl;
    else
        cout << "No client extensions." << endl << endl;

#ifdef EGL_EXT_device_base
    if (clientExts && strstr(clientExts, "EGL_EXT_device_base") != nullptr)
        printDevices();
#endif

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        cerr << "Could not obtain EGL display!" << endl;
        exit(1);
    }
    cout << "Default display" << endl;
    printDisplay(display);
}
