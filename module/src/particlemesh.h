#pragma once
#include <mesh.h>

namespace nap
{
	// Forward Declares
	class Core;
	class RenderService;

	class ParticleMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		// Constructor
		ParticleMesh(Core& core);

		virtual bool init(utility::ErrorState& errorState) override;

		void setCount(uint count)											{ mCount = count; }
		uint getCount() const												{ return mCount; }

		virtual MeshInstance& getMeshInstance() override					{ return *mMeshInstance; }
		virtual const MeshInstance& getMeshInstance() const override		{ return *mMeshInstance; }

		EMemoryUsage	mUsage = EMemoryUsage::Static;				///< Property: 'Usage' If the plane is uploaded once or frequently updated.
		ECullMode		mCullMode = ECullMode::None;				///< Property: 'CullMode' Plane cull mode, defaults to no culling
		EPolygonMode	mPolygonMode = EPolygonMode::Fill;			///< Property: 'PolygonMode' Polygon rasterization mode (fill, line, points)
		uint			mCount = 32;

	private:
		std::unique_ptr<MeshInstance>	mMeshInstance = nullptr;	///< The mesh instance to construct
		nap::RenderService*				mRenderService = nullptr;	///< Handle to the render service
	};
}
