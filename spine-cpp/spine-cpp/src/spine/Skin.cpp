/******************************************************************************
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


#include <spine/Skin.h>

#include <spine/Attachment.h>
#include <spine/MeshAttachment.h>
#include <spine/Skeleton.h>

#include <spine/Slot.h>
#include <spine/ConstraintData.h>

#include <assert.h>

using namespace spine;


void Skin::AttachmentMap::put(int32 slotIndex, const FString &attachmentName, TSharedPtr<Attachment> attachment)
{
	if (slotIndex >= _buckets.Num())
	{
		_buckets.SetNum(slotIndex + 1);
	}
	TArray<Entry> &bucket = _buckets[slotIndex];
	int existing = findInBucket(bucket, attachmentName);

	if (existing >= 0)
	{
		bucket[existing]._attachment = attachment;
	}
	else
	{
		bucket.Add(Entry(slotIndex, attachmentName, attachment));
	}
}


TSharedPtr<Attachment> Skin::AttachmentMap::get(int32 slotIndex, const FString &attachmentName) 
{
	if (slotIndex >= _buckets.Num())
	{
		return NULL;
	}
	int existing = findInBucket(_buckets[slotIndex], attachmentName);
	return existing >= 0 ? _buckets[slotIndex][existing]._attachment : NULL;
}

void Skin::AttachmentMap::remove(int32 slotIndex, const FString &attachmentName) {
	if (slotIndex >= _buckets.Num())
	{
		return;
	}
	int existing = findInBucket(_buckets[slotIndex], attachmentName);
	if (existing >= 0)
	{
		_buckets[slotIndex].RemoveAt(existing);
	}
}

int Skin::AttachmentMap::findInBucket(TArray<Entry> &bucket, const FString &attachmentName) {
	for (int32 i = 0; i < bucket.Num(); i++)
	{
		if (bucket[i]._name == attachmentName)
		{
			return i;
		}
	}
	return -1;
}

Skin::AttachmentMap::Entries Skin::AttachmentMap::getEntries() 
{
	return Skin::AttachmentMap::Entries(_buckets);
}

Skin::Skin(const FString &name) : _name(name), _attachments() 
{
	assert(_name.length() > 0);
}

Skin::~Skin()
{
	/*Skin::AttachmentMap::Entries entries = _attachments.getEntries();
	while (entries.hasNext()) {
		Skin::AttachmentMap::Entry entry = entries.next();
		disposeAttachment(entry._attachment);
	}*/
}

void Skin::setAttachment(int32 slotIndex, const FString &name, TSharedPtr<Attachment> attachment) {
	check(attachment.IsValid());
	_attachments.put(slotIndex, name, attachment);
}

TSharedPtr<Attachment> Skin::getAttachment(int32 slotIndex, const FString &name) {
	return _attachments.get(slotIndex, name);
}


void Skin::removeAttachment(int32 slotIndex, const FString& name) {
	_attachments.remove(slotIndex, name);
}

void Skin::findNamesForSlot(int32 slotIndex, TArray<FString> &names) {
	Skin::AttachmentMap::Entries entries = _attachments.getEntries();
	while (entries.hasNext()) {
		Skin::AttachmentMap::Entry &entry = entries.next();
		if (entry._slotIndex == slotIndex) {
			names.Add(entry._name);
		}
	}
}

void Skin::findAttachmentsForSlot(int32 slotIndex, TArray<TSharedPtr<Attachment>> &attachments) {
	Skin::AttachmentMap::Entries entries = _attachments.getEntries();
	while (entries.hasNext()) {
		Skin::AttachmentMap::Entry &entry = entries.next();
		if (entry._slotIndex == slotIndex) {
			attachments.Add(entry._attachment);
		}
	}
}

const FString &Skin::getName() {
	return _name;
}

Skin::AttachmentMap::Entries Skin::getAttachments() {
	return _attachments.getEntries();
}

void Skin::attachAll(Skeleton &skeleton, Skin &oldSkin) 
{
	TArray<TSharedPtr<Slot>>  &slots = skeleton.getSlots();
	Skin::AttachmentMap::Entries entries = oldSkin.getAttachments();
	while (entries.hasNext())
	{
		Skin::AttachmentMap::Entry &entry = entries.next();
		int slotIndex = entry._slotIndex;
		TSharedPtr<Slot> slot = slots[slotIndex];

		if (slot->getAttachment() == entry._attachment.Get())
		{
			TSharedPtr<Attachment> attachment = getAttachment(slotIndex, entry._name);
			if (attachment) {
				slot->setAttachment(attachment.Get());
			}
		}
	}
}

TSharedRef<Skin> Skin::GetClone(const FString& NewName) const
{
	TSharedRef <Skin> NewSkin = MakeShared<Skin>(NewName);
	NewSkin->_attachments = this->_attachments;
	return NewSkin;
}
void Skin::addSkin(Skin* other) {
	for (int32 i = 0; i < other->getBones().size(); i++)
		if (!_bones.contains(other->getBones()[i])) _bones.add(other->getBones()[i]);

	for (int32 i = 0; i < other->getConstraints().size(); i++)
		if (!_constraints.contains(other->getConstraints()[i])) _constraints.add(other->getConstraints()[i]);

	AttachmentMap::Entries entries = other->getAttachments();
	while(entries.hasNext()) {
		AttachmentMap::Entry& entry = entries.next();
		setAttachment(entry._slotIndex, entry._name, entry._attachment);
	}
}

void Skin::copySkin(Skin* other) {
	for (int32 i = 0; i < other->getBones().size(); i++)
		if (!_bones.contains(other->getBones()[i])) _bones.add(other->getBones()[i]);

	for (int32 i = 0; i < other->getConstraints().size(); i++)
		if (!_constraints.contains(other->getConstraints()[i])) _constraints.add(other->getConstraints()[i]);

	AttachmentMap::Entries entries = other->getAttachments();
	while(entries.hasNext()) {
		AttachmentMap::Entry& entry = entries.next();
		if (entry._attachment->getRTTI().isExactly(MeshAttachment::rtti)) 
		{
			MeshAttachment* NewMeshAttach = static_cast<MeshAttachment*>(entry._attachment.Get())->newLinkedMesh();
			setAttachment(entry._slotIndex, entry._name,MakeShareable(NewMeshAttach) );
		} else 
		{
			Attachment* NewAttach = entry._attachment->copy();
			setAttachment(entry._slotIndex, entry._name, MakeShareable(NewAttach));
		}
	}
}

Vector<ConstraintData*>& Skin::getConstraints() {
	return _constraints;
}

Vector<BoneData*>& Skin::getBones() {
	return _bones;
}


