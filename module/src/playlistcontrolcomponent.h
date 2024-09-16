#pragma once

// Nap includes
#include <component.h>
#include <parameter.h>
#include <parametergroup.h>
#include <componentptr.h>
#include <parameterblendcomponent.h>

namespace nap
{
    class PlaylistControlComponentInstance;

    /**
     * Component that automatically selects presets on the ParameterBlendComponents
     * It cycles through a sequence of playlist items.
     * The order of the sequence can be shuffled and the duration of each preset can be randomized.
     */
    class NAPAPI PlaylistControlComponent : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(PlaylistControlComponent, PlaylistControlComponentInstance)
        
    public:

        void getDependentComponents(std::vector<rtti::TypeInfo> &components) const override;

        struct PresetGroup
        {
            ResourcePtr<ParameterGroup> mParameterGroup = nullptr;		// The parametergroup that contains the preset
            ResourcePtr<ParameterBlendComponent> mBlender = nullptr;	// The parameter blender that contains the parameter blend group
            std::string mPreset = "";								    // name of the json preset file
        };

        // Metadata about one preset in the sequence
        class NAPAPI Item : public Resource
        {
            RTTI_ENABLE(Resource)
        public:
            std::vector<PresetGroup> mPresets; // group of presets to blend
            float mAverageDuration = 5.f;	// average duration of the preset in the sequence in seconds
            float mDurationDeviation = 0.f;	// random deviation of the preset duration in seconds
            float mTransitionTime = 3.f;	// duration of the video fade into this preset in seconds

            bool init(utility::ErrorState& errorState) override;
        };

        std::vector<ResourcePtr<Item>> mItems;			// List of presets in the sequence accompanied by meta data
		bool mEnable;									// True to enable the preset cycle
        bool mRandomizePlaylist = false;				// Indicates whether the order of the cycle of presets will be shuffled
        bool mVerbose = true;							// Whether to log playlist changes
    };


    /**
     * Instance of @PlaylistControlComponent
     */
    class NAPAPI PlaylistControlComponentInstance : public ComponentInstance
    {
        RTTI_ENABLE(ComponentInstance)
    public:
        struct ItemPresetGroup
        {
            ItemPresetGroup(int index, ParameterGroup* group, ParameterBlendComponentInstance* blender, const std::string& preset);

            ParameterGroup* mParameterGroup = nullptr;
            ParameterBlendComponentInstance* mBlender;
            std::string mPreset = "";
            int mPresetIndex = 0;
        };

        struct Item
        {
            Item(){}
            Item(const PlaylistControlComponent::Item& resource, std::vector<ItemPresetGroup>& groups);

            std::vector<ItemPresetGroup> mGroups;
            float mAverageDuration = 5.f;
            float mDurationDeviation = 0.f;
            float mTransitionTime = 3.f;
            std::string mID;
        };

        PlaylistControlComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
        
        // Initialize the component
        bool init(utility::ErrorState& errorState) override;

        /**
         * Checks wether it is time to switch to the next preset and tells the @SwitchPresetComponent to switch.
         */
        void update(double deltaTime) override;

        /**
         * Manually sets playlist item to index
         * @param index index of playlist item
         */
        void setItem(int index);

		/**
		 * @return whether preset cycling is enabled
		 */
		bool isEnabled() const							{ return mResource->mEnable; }

        /**
         * Returns playlist
         * @return playlist
         */
        const std::vector<Item>& getPlaylist() const    { return mPlaylist; }

        /**
         * Returns current playlist index
         * @return current playlist index
         */
        int getCurrentPlaylistIndex() const            { return mCurrentPlaylistIndex; }
    private:
        void setItemInternal(int index, bool randomize);

        // Selects the next preset in the sequence
        void nextItem();

        // Permutes a list of presets. Helper method.
        void permute(std::vector<PlaylistControlComponentInstance::Item*>& list);

		PlaylistControlComponent* mResource = nullptr;

        std::vector<Item> mPlaylist;
        std::vector<Item*> mPermutedPlaylist;
        int mCurrentPlaylistIndex = -1;
        float mCurrentPlaylistItemDuration = 0;
        float mCurrentPlaylistItemElapsedTime = 0;
        Item* mCurrentPlaylistItem;

        bool mRandomizePlaylist = false;
        bool mVerbose = false;
    };
        
}
