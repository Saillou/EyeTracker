#ifndef CV_VIDEO_CAPTURE_PROPERTIES
#define CV_VIDEO_CAPTURE_PROPERTIES

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <string>
#include <memory>
#include <map>

namespace Dk {
namespace CvProperties {
	// ---------------------------- Structures ---------------------------------- 
	struct Property {
		int id;
		double value;
	};
	
	enum Type {
		Camera, Movie
	};
	
	// ------------------------------ Opencv explicitation -------------------------------- 
	const std::vector<int> GenerateAllPropertiesId() {
		return std::vector<int> {
			cv::CAP_PROP_POS_MSEC, 		// Current position of the video file in milliseconds.
			cv::CAP_PROP_POS_FRAMES, 	// 0-based index of the frame to be decoded/captured next.
			cv::CAP_PROP_POS_AVI_RATIO,	// Relative position of the video file: 0 - start of the film, 1 - end of the film.
			cv::CAP_PROP_FRAME_WIDTH, 	// Width of the frames in the video stream.
			cv::CAP_PROP_FRAME_HEIGHT, 	// Height of the frames in the video stream.
			cv::CAP_PROP_FPS, 			// Frame rate.
			cv::CAP_PROP_FOURCC, 		// 4-character code of codec.
			cv::CAP_PROP_FRAME_COUNT, 	// Number of frames in the video file.
			cv::CAP_PROP_FORMAT, 		// Format of the Mat objects returned by retrieve() .
			cv::CAP_PROP_MODE, 			// Backend-specific value indicating the current capture mode.
			cv::CAP_PROP_BRIGHTNESS, 	// Brightness of the image (only for cameras).
			cv::CAP_PROP_CONTRAST, 		// Contrast of the image (only for cameras).
			cv::CAP_PROP_SATURATION, 	// Saturation of the image (only for cameras).
			cv::CAP_PROP_HUE, 			// Hue of the image (only for cameras).
			cv::CAP_PROP_GAIN, 			// Gain of the image (only for cameras).
			cv::CAP_PROP_EXPOSURE, 		// Exposure (only for cameras).
			cv::CAP_PROP_CONVERT_RGB, 	// Boolean flags indicating whether images should be converted to RGB.
			cv::CAP_PROP_WHITE_BALANCE_BLUE_U,
			cv::CAP_PROP_RECTIFICATION,	// Rectification flag for stereo cameras (note: only supported by DC1394 v 2.x backend currently)
			cv::CAP_PROP_MONOCHROME,
			cv::CAP_PROP_SHARPNESS,
			cv::CAP_PROP_AUTO_EXPOSURE,
			cv::CAP_PROP_GAMMA,
			cv::CAP_PROP_TEMPERATURE,
			cv::CAP_PROP_TRIGGER,
			cv::CAP_PROP_TRIGGER_DELAY,
			cv::CAP_PROP_WHITE_BALANCE_RED_V,
			cv::CAP_PROP_ZOOM,
			cv::CAP_PROP_FOCUS,
			cv::CAP_PROP_GUID,
			cv::CAP_PROP_ISO_SPEED,
			cv::CAP_PROP_BACKLIGHT,
			cv::CAP_PROP_PAN,
			cv::CAP_PROP_TILT,
			cv::CAP_PROP_ROLL,
			cv::CAP_PROP_IRIS,
			cv::CAP_PROP_SETTINGS,
			cv::CAP_PROP_BUFFERSIZE,
			cv::CAP_PROP_AUTOFOCUS,
		};	
	}
	const std::string GetPropertyName(int id) {
		switch(id) {
			case cv::CAP_PROP_POS_MSEC: 		return "CAP_PROP_POS_MSEC";
			case cv::CAP_PROP_POS_FRAMES: 		return "CAP_PROP_POS_FRAMES";
			case cv::CAP_PROP_POS_AVI_RATIO: 	return "CAP_PROP_POS_AVI_RATIO";
			case cv::CAP_PROP_FRAME_WIDTH: 		return "CAP_PROP_FRAME_WIDTH";
			case cv::CAP_PROP_FRAME_HEIGHT: 	return "CAP_PROP_FRAME_HEIGHT";
			case cv::CAP_PROP_FPS: 				return "CAP_PROP_FPS";
			case cv::CAP_PROP_FOURCC: 			return "CAP_PROP_FOURCC";
			case cv::CAP_PROP_FRAME_COUNT: 		return "CAP_PROP_FRAME_COUNT";
			case cv::CAP_PROP_FORMAT: 			return "CAP_PROP_FORMAT";
			case cv::CAP_PROP_MODE: 			return "CAP_PROP_MODE";
			case cv::CAP_PROP_BRIGHTNESS: 		return "CAP_PROP_BRIGHTNESS";
			case cv::CAP_PROP_CONTRAST: 		return "CAP_PROP_CONTRAST";
			case cv::CAP_PROP_SATURATION: 		return "CAP_PROP_SATURATION";
			case cv::CAP_PROP_HUE: 				return "CAP_PROP_HUE";
			case cv::CAP_PROP_GAIN: 			return "CAP_PROP_GAIN";
			case cv::CAP_PROP_EXPOSURE: 		return "CAP_PROP_EXPOSURE";
			case cv::CAP_PROP_CONVERT_RGB: 		return "CAP_PROP_CONVERT_RGB";
			case cv::CAP_PROP_WHITE_BALANCE_BLUE_U: return "CAP_PROP_WHITE_BALANCE_BLUE_U";
			case cv::CAP_PROP_RECTIFICATION: 	return "CAP_PROP_RECTIFICATION"; 
			case cv::CAP_PROP_MONOCHROME:		return "CAP_PROP_MONOCHROME";
			case cv::CAP_PROP_SHARPNESS: 		return "CAP_PROP_SHARPNESS";
			case cv::CAP_PROP_AUTO_EXPOSURE: 	return "CAP_PROP_AUTO_EXPOSURE";
			case cv::CAP_PROP_GAMMA: 			return "CAP_PROP_GAMMA";
			case cv::CAP_PROP_TEMPERATURE: 		return "CAP_PROP_TEMPERATURE";
			case cv::CAP_PROP_TRIGGER: 			return "CAP_PROP_TRIGGER";
			case cv::CAP_PROP_TRIGGER_DELAY: 	return "CAP_PROP_TRIGGER_DELAY";
			case cv::CAP_PROP_WHITE_BALANCE_RED_V: return "CAP_PROP_WHITE_BALANCE_RED_V";
			case cv::CAP_PROP_ZOOM: 			return "CAP_PROP_ZOOM";
			case cv::CAP_PROP_FOCUS: 			return "CAP_PROP_FOCUS";
			case cv::CAP_PROP_GUID: 			return "CAP_PROP_GUID";
			case cv::CAP_PROP_ISO_SPEED: 		return "CAP_PROP_ISO_SPEED";
			case cv::CAP_PROP_BACKLIGHT: 		return "CAP_PROP_BACKLIGHT";
			case cv::CAP_PROP_PAN: 				return "CAP_PROP_PAN";
			case cv::CAP_PROP_TILT: 			return "CAP_PROP_TILT";
			case cv::CAP_PROP_ROLL: 			return "CAP_PROP_ROLL";
			case cv::CAP_PROP_IRIS: 			return "CAP_PROP_IRIS";
			case cv::CAP_PROP_SETTINGS: 		return "CAP_PROP_SETTINGS";
			case cv::CAP_PROP_BUFFERSIZE: 		return "CAP_PROP_BUFFERSIZE";
			case cv::CAP_PROP_AUTOFOCUS: 		return "CAP_PROP_AUTOFOCUS";
  
			default: return "";
		}
	}
	
	
	// -------------------------------- Helper Methods ---------------------------------- 
	namespace Helper {	
		Property Get(const cv::VideoCapture& cap, int property_id) {
			Property prop;
			prop.id 	= property_id;
			prop.value 	= cap.get(property_id);
			
			return prop;
		}
		bool Set(cv::VideoCapture& cap, const Property& prop) {
			return cap.set(prop.id, prop.value);
		}
		
		std::map<int, Property> GetAll(const cv::VideoCapture& cap) {			
			std::map<int, Property> mappedProp;
			for(int idProp : GenerateAllPropertiesId()) 
				mappedProp[idProp] = Get(cap, idProp);
			
			return mappedProp;
		}
	} // Helper
	
	
	// ------------------------------ Packaging ---------------------------------- 
	class CaptureProperties {
	public:
		// Constructors
		CaptureProperties(std::shared_ptr<cv::VideoCapture> pCam, Type type = Camera) : 
			_pCam(pCam), 
			_type(type),
			_pProperties(Helper::GetAll(*_pCam)) 
		{
		}
		
		// Methods
		const Property& get(int cvPropertyId, bool update = false) {			
			if(update)
				_pProperties[cvPropertyId] = Helper::Get(*_pCam, cvPropertyId);
			
			return _pProperties[cvPropertyId];
		}
		bool set(const Property& prop) {
			if(!_pCam)
				return false;
			
			bool res = Helper::Set(*_pCam, prop);
			
			// Update memory
			if(res)
				_pProperties[prop.id] = prop;
			
			return res;
		}
		bool set(int cvPropertyId, double manualValue) {
			auto prop = _pProperties[cvPropertyId];
			prop.value = manualValue;
			return set(prop);
		}
		
		// Getters
		const std::map<int, Property>& getAll() const {
			return _pProperties;
		}
		
	private:
		// Datas
		std::shared_ptr<cv::VideoCapture> _pCam;
		Type _type;
		std::map<int, Property> _pProperties;
	};
	
} // CvVCP
} // Dk

#endif