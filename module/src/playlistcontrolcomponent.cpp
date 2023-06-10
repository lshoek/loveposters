#include "playlistcontrolcomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>
#include <mathutils.h>
#include <nap/logger.h>

// RTTI

RTTI_BEGIN_STRUCT(nap::PlaylistControlComponent::PresetGroup)
    RTTI_PROPERTY_FILELINK("Preset", &nap::PlaylistControlComponent::PresetGroup::mPreset, nap::rtti::EPropertyMetaData::Default, nap::rtti::EPropertyFileType::Any)
    RTTI_PROPERTY("ParameterGroup", &nap::PlaylistControlComponent::PresetGroup::mParameterGroup, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Blender", &nap::PlaylistControlComponent::PresetGroup::mBlender, nap::rtti::EPropertyMetaData::Required)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::PlaylistControlComponent::Item)
    RTTI_PROPERTY("Groups", &nap::PlaylistControlComponent::Item::mPresets, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AverageDuration", &nap::PlaylistControlComponent::Item::mAverageDuration, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DurationDeviation", &nap::PlaylistControlComponent::Item::mDurationDeviation, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("TransitionTime", &nap::PlaylistControlComponent::Item::mTransitionTime, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::PlaylistControlComponent)
	RTTI_PROPERTY("Items", &nap::PlaylistControlComponent::mItems, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("RandomizePlaylist", &nap::PlaylistControlComponent::mRandomizePlaylist, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Enable", &nap::PlaylistControlComponent::mEnable, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Verbose", &nap::PlaylistControlComponent::mVerbose, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PlaylistControlComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // PlaylistControlComponent
    //////////////////////////////////////////////////////////////////////////

    void PlaylistControlComponent::getDependentComponents(std::vector<rtti::TypeInfo> &components) const
    {
        components.emplace_back(RTTI_OF(ParameterBlendComponent));
    }

    //////////////////////////////////////////////////////////////////////////
    // PlaylistControlComponent::Item
    //////////////////////////////////////////////////////////////////////////

    bool PlaylistControlComponent::Item::init(utility::ErrorState &errorState)
    {
        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    // PlaylistControlComponentInstance::ItemPresetGroup
    //////////////////////////////////////////////////////////////////////////

    PlaylistControlComponentInstance::ItemPresetGroup::ItemPresetGroup(int index,
                                                                       ParameterGroup* group,
                                                                       ParameterBlendComponentInstance* blender,
                                                                       const std::string& preset)
    {
        mPresetIndex = index;
        mParameterGroup = group;
        mBlender = blender;
        mPreset = preset;
    }

    //////////////////////////////////////////////////////////////////////////
    // PlaylistControlComponentInstance::Item
    //////////////////////////////////////////////////////////////////////////

    PlaylistControlComponentInstance::Item::Item(const PlaylistControlComponent::Item& resource,
                                                 std::vector<ItemPresetGroup>& groups)
    {
        mAverageDuration = resource.mAverageDuration;
        mDurationDeviation = resource.mDurationDeviation;
        mTransitionTime = resource.mTransitionTime;
        mGroups = groups;
        mID = resource.mID;
    }

    //////////////////////////////////////////////////////////////////////////
    // PlaylistControlComponentInstance
    //////////////////////////////////////////////////////////////////////////

	bool PlaylistControlComponentInstance::init(utility::ErrorState& errorState)
	{
        // Utility function to get root entity
        static auto find_root_entity =[](EntityInstance* entity)->EntityInstance*
        {
            EntityInstance* last_entity = entity;
            while (entity!= nullptr)
            {
                last_entity = entity;
                entity = entity->getParent();
            }

            return last_entity;
        };

        // Get entity instance
        auto* root_entity = find_root_entity(getEntityInstance());

        // Fetch resource
		mResource = getComponent<PlaylistControlComponent>();

        // Check if we have any presets
        if(!errorState.check(mPlaylist.empty(), "No playlist created"))
            return false;

        // Gather all blenders in scene
        std::vector<ParameterBlendComponentInstance*> blenders;
        root_entity->getComponentsOfTypeRecursive(blenders);

        // utility function to find ParameterBlendComponentInstance* matched to ParameterBlendComponent*
        static auto find_blender = [](const ParameterBlendComponent* component,
                                      const std::vector<ParameterBlendComponentInstance*> blenders)->ParameterBlendComponentInstance*
        {
            for(auto& blender_instance : blenders)
            {
                if(blender_instance->getComponent()->mID==component->mID)
                    return blender_instance;
            }

            return nullptr;
        };

        // create the preset info's
        for(auto& preset : mResource->mItems)
        {
            std::vector<ItemPresetGroup> preset_groups;
            for(auto& group : preset->mPresets)
            {
                // find the blender instance
                auto* blender_instance = find_blender(group.mBlender.get(), blenders);
                if(!errorState.check(blender_instance!= nullptr, "Could not find instance for blender %s",
                                     group.mBlender->mID.c_str()))
                    return false;

                // find preset index
                bool found = false;
                int idx = 0;
                const auto& blender_instance_presets = blender_instance->getPresets();
                for(const auto& blender_instance_preset : blender_instance_presets)
                {
                    if(blender_instance_preset==utility::getFileName(group.mPreset))
                    {
                        found = true;
                        break;
                    }else
                    {
                        idx++;
                    }
                }
                if(!errorState.check(found, "Could not find preset %s in blender %s",
                                    group.mPreset.c_str(), blender_instance->mID.c_str()))
                    return false;
                preset_groups.emplace_back(ItemPresetGroup(idx, group.mParameterGroup.get(), blender_instance, group.mPreset));
            }

            mPlaylist.emplace_back(Item(*preset.get(), preset_groups));
        }

		// Exit early if there are no items
		if (mPlaylist.empty())
			return true;

        for(auto& item : mPlaylist)
        {
            for(auto& group : item.mGroups)
            {
                // Find presets on disk
                auto* param_service = getEntityInstance()->getCore()->getService<nap::ParameterService>();
                std::vector<std::string> presets = param_service->getPresets(*group.mParameterGroup);
                if (!errorState.check(presets.size() > 0, "%s: No presets available", mID.c_str()))
                    return false;
            }
        }

		for (auto& preset : mPlaylist)
			mPermutedPlaylist.emplace_back(&preset);
		permute(mPermutedPlaylist);

        mRandomizePlaylist = mResource->mRandomizePlaylist;
        mVerbose = mResource->mVerbose;

        mCurrentPlaylistItemDuration = mPlaylist[0].mAverageDuration;
        mCurrentPlaylistItemElapsedTime = 0;
        mCurrentPlaylistIndex = 0;

		if (isEnabled())
        {
            mCurrentPlaylistItem = &mPlaylist[mCurrentPlaylistIndex];

            for(auto& group : mCurrentPlaylistItem->mGroups)
            {
                group.mBlender->getComponent<ParameterBlendComponent>()->mPresetIndex->setValue(group.mPresetIndex);
            }
        }

		return true;
	}


	void PlaylistControlComponentInstance::update(double deltaTime)
	{
		if (!isEnabled() || mPlaylist.empty())
			return;

		mCurrentPlaylistItemElapsedTime += deltaTime;
		if (mCurrentPlaylistItemElapsedTime >= mCurrentPlaylistItemDuration)
            nextItem();
	}


	void PlaylistControlComponentInstance::nextItem()
	{
		mCurrentPlaylistIndex++;
		if (mCurrentPlaylistIndex >= mPlaylist.size())
		{
            if(mRandomizePlaylist)
			    permute(mPermutedPlaylist);
			mCurrentPlaylistIndex = 0;
		}
        setItemInternal(mCurrentPlaylistIndex, mRandomizePlaylist);
	}


    void PlaylistControlComponentInstance::setItem(int index)
    {
        if(index >= 0 && index < mPlaylist.size())
        {
            setItemInternal(index, false);
        }else
        {
            nap::Logger::error(*this, "Wrong index %i", index);
        }
    }


    void PlaylistControlComponentInstance::setItemInternal(int index, bool randomize)
    {
        assert(index >= 0 && index < mPlaylist.size());

        mCurrentPlaylistIndex = index;

        auto item = &mPlaylist[mCurrentPlaylistIndex];
        if (randomize)
            item = mPermutedPlaylist[mCurrentPlaylistIndex];

        mCurrentPlaylistItemDuration = item->mAverageDuration + math::random(-item->mDurationDeviation / 2.f, item->mDurationDeviation / 2.f);
        mCurrentPlaylistItemElapsedTime = 0;
        mCurrentPlaylistItem = item;

        for(auto& group : item->mGroups)
        {
            auto* blender = group.mBlender;
            blender->getComponent<ParameterBlendComponent>()->mPresetIndex->setValue(group.mPresetIndex);
            blender->getComponent<ParameterBlendComponent>()->mPresetBlendTime->setValue(mPlaylist[mCurrentPlaylistIndex].mTransitionTime);
        }

        if(mVerbose)
        {
            nap::Logger::info(*this, "Switching to playlist item %s", item->mID.c_str());
        }
    }


	void PlaylistControlComponentInstance::permute(std::vector<PlaylistControlComponentInstance::Item*>& list)
	{
		for (auto i = 0; i < list.size(); ++i)
		{
			auto swapIndex = math::random<int>(0, list.size() - 1);
			auto temp = list[swapIndex];
			list[swapIndex] = list[i];
			list[i] = temp;
		}
	}

}
