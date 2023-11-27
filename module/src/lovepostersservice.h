#pragma once

// External Includes
#include <nap/service.h>
#include <parametergroup.h>

namespace nap
{
	class NAPAPI LovePostersService : public Service
	{
		RTTI_ENABLE(Service)
	public:
		// Default Constructor
		LovePostersService(ServiceConfiguration* configuration) : Service(configuration)	{ }

		/**
		 * Use this call to register service dependencies
		 * A service that depends on another service is initialized after all it's associated dependencies
		 * This will ensure correct order of initialization, update calls and shutdown of all services
		 * @param dependencies rtti information of the services this service depends on
		 */
		virtual void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;

		/**
		 * Initializes the service
		 * @param errorState contains the error message on failure
		 * @return if the video service was initialized correctly
		 */
		virtual bool init(nap::utility::ErrorState& errorState) override;

    protected:
        void registerObjectCreators(rtti::Factory &factory) override;
	};
}
