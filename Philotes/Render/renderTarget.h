
#ifndef RENDERER_TARGET_H
#define RENDERER_TARGET_H

_NAMESPACE_BEGIN

class RendererTarget
{
	friend class Renderer;
	public:
		RendererTarget(void);
		virtual ~RendererTarget(void);
		
		void release(void) 
		{ 
			delete this; 
		}
	
	private:
		virtual void bind(void) = 0;

		virtual void unbind(void) = 0;
};

_NAMESPACE_END

#endif