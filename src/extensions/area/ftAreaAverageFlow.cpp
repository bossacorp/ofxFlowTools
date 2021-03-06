#include "ftAreaAverageFlow.h"

namespace flowTools {
	
	int ftAreaAverageFlow::areaCount = 0;
	
	void ftAreaAverageFlow::setup(int _width, int _height, ftFlowForceType _type) {
		areaCount++;
		type = _type;
		GLint internalFormat = ftUtil::getInternalFormatFromType(type);
		ftFlow::allocate(_width, _height, internalFormat);
		roiFbo.allocate(_width, _height, internalFormat);
		ftUtil::zero(roiFbo);
		numChannels = ftUtil::getNumChannelsFromInternalFormat(internalFormat);
		
		direction.clear();
		direction.resize(numChannels, 0);
		components.clear();
		components.resize(numChannels, 0);
		
		inputPixels.allocate(inputWidth, inputHeight, numChannels);
		magnitudes.resize(inputWidth * inputHeight, 0);
		
		roi = ofRectangle(0,0,1,1);
		
		meanMagnitude = 0;
		stdevMagnitude = 0;
		
		parameters.setName("area " + ftFlowForceNames[type] + " " + ofToString(areaCount));
		
		parameters.add(pMeanMagnitude.set("mean mag", 0, 0, 1));
//		parameters.add(pStdevMagnitude.set("stdev mag", 0, 0, 1));
				
		pComponents.resize(numChannels);
		pDirection.resize(numChannels);
		if (numChannels > 1) {
			componentParameters.setName("components");
			directionParameters.setName("direction");
			for (int i=0; i<numChannels; i++) {
				componentParameters.add(pComponents[i].set(getComponentName(i), 0, -1, 1));
//				directionParameters.add(pDirection[i].set(getComponentName(i), 0, -1, 1));
			}
			parameters.add(componentParameters);
//			parameters.add(directionParameters);
		} else {
			parameters.add(pComponents[0].set(getComponentName(0), 0, -1, 1));
//			parameters.add(pDirection[0].set(componentNames[0], 0, -1, 1));
		}
		parameters.add(pNormalizationMax.set("normalization max", .25, .01, 1));
		parameters.add(pHighComponentBoost.set("boost highest comp.", 0, 0, 5));
		
		roiParameters.setName("ROI");
		pRoi.resize(4);
		roiParameters.add(pRoi[0].set("x", 0, 0, 1));
		roiParameters.add(pRoi[1].set("y", 0, 0, 1));
		roiParameters.add(pRoi[2].set("width", 1, 0, 1));
		roiParameters.add(pRoi[3].set("height", 1, 0, 1));
		for (int i=0; i<4; i++) {
			pRoi[i].addListener(this, &ftAreaAverageFlow::pRoiListener);
		}
		parameters.add(roiParameters);
	}
	
	void ftAreaAverageFlow::setInput(ofTexture &_tex){
		resetInput();
		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		ftUtil::roi(inputFbo, _tex, roi);
		ofPopStyle();
	}
	
	void ftAreaAverageFlow::addInput(ofTexture &_tex, float _strength) {
		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		ftUtil::zero(roiFbo);
		ftUtil::roi(roiFbo, _tex, roi);
		ofPopStyle();
		ftFlow::addInput(roiFbo.getTexture(), _strength);
	}

	void ftAreaAverageFlow::update() {
		ftUtil::toPixels(inputFbo, inputPixels);
		float* floatPixelData = inputPixels.getData();
		
		vector<float> totalVelocity;
		totalVelocity.resize(numChannels, 0);
		
		int numPixels = inputWidth * inputHeight;
		for (int i=0; i<numPixels; i++) {
			float mag = 0;
			for (int j=0; j<numChannels; j++) {
				float vel = floatPixelData[i * numChannels + j];
				totalVelocity[j] += vel;
				mag += vel * vel;
			}
			magnitudes[i] = sqrt(mag);
		}
		getMeanStDev(magnitudes, meanMagnitude, stdevMagnitude);
		
		meanMagnitude = meanMagnitude / pNormalizationMax.get();
		meanMagnitude = ofClamp(meanMagnitude, 0, 1);
		
		float totalMagnitude;
		for (auto tv : totalVelocity) { totalMagnitude += tv * tv; }
		totalMagnitude = sqrt(totalMagnitude);
		
		for (int i=0; i<numChannels; i++) {
			direction[i] = totalVelocity[i] / totalMagnitude;
			components[i] = direction[i] * meanMagnitude;
		}
		
		if (pHighComponentBoost.get() > 0 && numChannels > 1) {
			// normalize to highest component and apply boost
			float highVelocity = 0;
			float P = 1;
			for (int i=0; i<numChannels; i++) {
				if (fabs(components[i]) > highVelocity) {
					highVelocity = fabs(components[i]);
					if (components[i] < 0) P = -1;
				}
			}
			for (int i=0; i<numChannels; i++) {
				components[i] /= highVelocity;
				components[i] = powf(fabs(components[i]), pHighComponentBoost.get()) * P;
				components[i] *= highVelocity;
			}
		}
		
		for (int i=0; i<numChannels; i++) {
			pComponents[i] = int(components[i] * 100) / 100.0;
			pDirection[i] = int(direction[i] * 100) / 100.0;
		}
		pMeanMagnitude.set(int(meanMagnitude * 100) / 100.0);
		pStdevMagnitude.set(int(stdevMagnitude * 100) / 100.0);
	}
	
	void ftAreaAverageFlow::drawOutput(int _x, int _y, int _w, int _h) {
		int x = _x + roi.x * _w;
		int y = _y + roi.y * _h;
		int w = roi.width * _w;
		int h = roi.height * _h;
		ofPushStyle();
		ofSetColor(128, 128, 128, 64);
		ofDrawRectangle(x, y, w, h);
		ofPopStyle();
	}
	
	void ftAreaAverageFlow::setRoi(ofRectangle _rect) {
		float x = _rect.x;
		float y = _rect.y;
		float maxW = 1.0 - x;
		float maxH = 1.0 - y;
		float w = min(_rect.width, maxW);
		float h = min(_rect.height, maxH);
		
		roi = ofRectangle(x, y, w, h);
		
		if (pRoi[0] != x) { pRoi[0].set(x); }
		if (pRoi[1] != y) { pRoi[1].set(y); }
		if (pRoi[2].getMax() != maxW) { pRoi[2].setMax(maxW); pRoi[2].set(w); }
		if (pRoi[3].getMax() != maxH) { pRoi[3].setMax(maxH); pRoi[3].set(h); }
		if (pRoi[2] != w) { pRoi[2].set(w); }
		if (pRoi[3] != h) { pRoi[3].set(h); }
	}
	
	void ftAreaAverageFlow::getMeanStDev(vector<float> &_v, float &_mean, float &_stDev) {
		float mean = accumulate(_v.begin(), _v.end(), 0.0) / (float)_v.size();
		std::vector<float> diff(_v.size());
		std::transform(_v.begin(), _v.end(), diff.begin(), std::bind2nd(std::minus<float>(), mean));
		float sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
		float stDev = std::sqrt(sq_sum / _v.size());
		
		_mean = mean;
		_stDev = stDev;
	}
	
	string ftAreaAverageFlow::getComponentName(int _index)  {
		vector<string> componentNames;
		switch (type) {
			case FT_VELOCITY:
			case FT_VELOCITY_NORM:
				componentNames = {"x", "y"};
				break;
			case FT_VELOCITY_SPLIT:
				componentNames = {"right", "down", "left", "up"};
				break;
			case FT_DENSITY:
				componentNames = {"red", "green", "blue", "alpha"};
				break;
			case FT_PRESSURE:
				componentNames = {"pressure"};
				break;
			case FT_TEMPERATURE:
				componentNames = {"temperature"};
				break;
			default:
				componentNames = {"unknown 0", "unknown 1", "unknown 2", "unknown 3"};
				break;
		}
		
		if (_index < componentNames.size()) {
			return componentNames[_index];
		}
		return "unknown";
		
	}
}
