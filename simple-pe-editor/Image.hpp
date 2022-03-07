#ifndef __IMAGE__H__

#define __IMAGE__H__

class Image
{
protected:
	Image() = default;
public:
	virtual void parse(const char filename[]) = 0;
	virtual bool isLoaded() = 0;
	virtual void release() = 0;
};

#endif
