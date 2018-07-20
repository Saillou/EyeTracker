#ifndef CHRONOMETRE_H
#define CHRONOMETRE_H

#include <chrono>

struct Chronometre {	
public:
	Chronometre():
		_tRef(_now()),
		_tBeg(_tRef),
		_tEnd(_tRef)
	{
	}

	// Methods	
	void beg() {
		_tBeg = _now();
	}
	void end() {
		_tEnd = _now();
	}
	void reset() {
		_tRef = _now();
		_tBeg = _tRef;
		_tEnd = _tRef;
	}
	
	int64_t clock_ms() const {
		return  _diffMs(_tRef, _now());
	}
	int64_t ms() const {
		return _diffMs(_tBeg, _tEnd);
	}
	int64_t elapsed_ms() const {
		return _diffMs(_tBeg, _now());
	}
	
	// Static methods
	static void wait(int ms) {
		_timePoint c0 = _now();
		while(_diffMs(c0, _now()) < ms);
	}
	
private:
	typedef std::chrono::high_resolution_clock::time_point 	_timePoint;
	
	// Static methods
	static _timePoint _now() {
		return std::chrono::high_resolution_clock::now();
	}
	static int64_t _diffMs(const _timePoint& tA, const _timePoint& tB) {
		return std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(tB - tA).count();
	}
	
	// Members
	_timePoint _tRef; // Memorise time of the chrono's birth / last reset
	_timePoint _tBeg;
	_timePoint _tEnd;
};

#endif