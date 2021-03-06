﻿/******************************************************************************
 * Spine Runtimes Software License v2.5
 *
 * Copyright (c) 2013-2016, Esoteric Software
 * All rights reserved.
 *
 * You are granted a perpetual, non-exclusive, non-sublicensable, and
 * non-transferable license to use, install, execute, and perform the Spine
 * Runtimes software and derivative works solely for personal or internal
 * use. Without the written permission of Esoteric Software (see Section 2 of
 * the Spine Software License Agreement), you may not (a) modify, translate,
 * adapt, or develop new applications using the Spine Runtimes or otherwise
 * create derivative works or improvements of the Spine Runtimes or (b) remove,
 * delete, alter, or obscure any trademarks or any copyright, trademark, patent,
 * or other intellectual property or proprietary rights notices on or in the
 * Software, including any copy thereof. Redistributions in binary or source
 * form must include this license and terms.
 *
 * THIS SOFTWARE IS PROVIDED BY ESOTERIC SOFTWARE "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL ESOTERIC SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES, BUSINESS INTERRUPTION, OR LOSS OF
 * USE, DATA, OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#pragma once

#include "CoreMinimal.h"

#include <spine/Vector.h>

namespace spine 
{
	class Attachment;

class Skeleton;
class BoneData;
class ConstraintData;

	/// Stores attachments by slot index and attachment name.
	/// See SkeletonData::getDefaultSkin, Skeleton::getSkin, and
	/// http://esotericsoftware.com/spine-runtime-skins in the Spine Runtimes Guide.


	class SP_API Skin :public TSharedFromThis<Skin>
	{
		friend class Skeleton;

	public:
		class SP_API AttachmentMap
		{
			friend class Skin;

		public:
			struct SP_API Entry
			{
				int32 _slotIndex;
				FString _name;
				TSharedPtr<Attachment> _attachment;

				Entry(int32 slotIndex, const FString &name, TSharedPtr<Attachment> attachment) :
					_slotIndex(slotIndex),
					_name(name),
					_attachment(attachment)
				{
				}
			};

			class SP_API Entries
			{
				friend class AttachmentMap;

			public:
				bool hasNext()
				{
					while (true) 
					{
						if (_slotIndex >= _buckets.Num()) return false;
						if (_bucketIndex >= _buckets[_slotIndex].Num()) 
						{
							_bucketIndex = 0;
							++_slotIndex;
							continue;
						};
						return true;
					}
				}

				Entry &next() 
				{
					Entry &result = _buckets[_slotIndex][_bucketIndex];
					++_bucketIndex;
					return result;
				}

			protected:
				Entries(TArray< TArray<Entry> > &buckets) : _buckets(buckets), _slotIndex(0), _bucketIndex(0)
				{
				}

			private:
				TArray< TArray<Entry> > &_buckets;
				int32 _slotIndex;
				int32 _bucketIndex;
			};

			void put(int32 slotIndex, const FString &attachmentName, TSharedPtr<Attachment> attachment);

			TSharedPtr<Attachment> get(int32 slotIndex, const FString &attachmentName);

			void remove(int32 slotIndex, const FString &attachmentName);

			Entries getEntries();

		protected:
			AttachmentMap(){}

		private:

			int findInBucket(TArray <Entry> &, const FString &attachmentName);

			TArray <TArray<Entry> > _buckets;
		};

		explicit Skin(const FString &name);

		~Skin();

	/// Adds an attachment to the skin for the specified slot index and name.
	/// If the name already exists for the slot, the previous value is replaced.
	void setAttachment(int32 slotIndex, const FString &name, TSharedPtr<Attachment> attachment);

	TSharedPtr<Attachment> getAttachment(int32 slotIndex, const FString &name);

	// Removes the attachment from the skin.
	void removeAttachment(int32 slotIndex, const FString& name);


	/// Finds the skin keys for a given slot. The results are added to the passed array of names.
	/// @param slotIndex The target slotIndex. To find the slot index, use Skeleton::findSlotIndex or SkeletonData::findSlotIndex
	/// @param names Found skin key names will be added to this array.
	void findNamesForSlot(int32 slotIndex, TArray <FString> &names);

	/// Finds the attachments for a given slot. The results are added to the passed array of Attachments.
	/// @param slotIndex The target slotIndex. To find the slot index, use Skeleton::findSlotIndex or SkeletonData::findSlotIndex
	/// @param attachments Found Attachments will be added to this array.
	void findAttachmentsForSlot(int32 slotIndex, TArray<TSharedPtr<Attachment>> &attachments);

	const FString &getName();

	/// Adds all attachments, bones, and constraints from the specified skin to this skin.
	void addSkin(Skin* other);

	/// Adds all attachments, bones, and constraints from the specified skin to this skin. Attachments are deep copied.
	void copySkin(Skin* other);

	AttachmentMap::Entries getAttachments();

	Vector<BoneData*>& getBones();

	Vector<ConstraintData*>& getConstraints();

	TSharedRef<Skin> GetClone(const FString& NewName)const;
private:
	
	FString _name;
	AttachmentMap _attachments;
	Vector<BoneData*> _bones;
	Vector<ConstraintData*> _constraints;

	/// Attach all attachments from this skin if the corresponding attachment from the old skin is currently attached.
	void attachAll(Skeleton &skeleton, Skin &oldSkin);
	};
}

