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
	struct Value {
		bool automatique;
		double manual;
	};
	struct Availability {
		bool manual;
		bool automatique;
	};
	struct Property {
		int id;
		Availability availability;
		Value value;
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
			cv::CAP_PROP_RECTIFICATION,	// Rectification flag for stereo cameras (note: only supported by DC1394 v 2.x backend currently)
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
			case cv::CAP_PROP_RECTIFICATION: 	return "CAP_PROP_RECTIFICATION";
			
			default: return "";
		}
	}
	
	
	// -------------------------------- Helper Methods ---------------------------------- 
	namespace Helper {	
		Property Get(const cv::VideoCapture& cap, int property_id) {
			Property prop;
			prop.id 						= property_id;
			prop.value.manual 				= cap.get(property_id);
			prop.value.automatique 			= false;
			prop.availability.manual 		= (prop.value.manual >= 0);
			prop.availability.automatique 	= false;
			
			return prop;
		}
		bool Set(cv::VideoCapture& cap, const Property& prop) {
			bool ret = cap.set(prop.id, prop.value.manual);
			
			return ret;
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
			if(!_pCam)
				return false;
			
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