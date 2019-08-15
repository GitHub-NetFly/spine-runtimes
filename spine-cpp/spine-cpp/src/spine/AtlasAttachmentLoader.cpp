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
******************************************************S**********************/



#include <spine/AtlasAttachmentLoader.h>
#include <spine/Skin.h>
#include <spine/RegionAttachment.h>
#include <spine/MeshAttachment.h>
#include <spine/BoundingBoxAttachment.h>
#include <spine/PathAttachment.h>
#include <spine/PointAttachment.h>
#include <spine/ClippingAttachment.h>

#include <spine/Atlas.h>

namespace spine {
RTTI_IMPL(AtlasAttachmentLoader, AttachmentLoader)

AtlasAttachmentLoader::AtlasAttachmentLoader(Atlas *atlas) : AttachmentLoader(), _atlas(atlas) {
}

TSharedPtr < RegionAttachment> AtlasAttachmentLoader::newRegionAttachment(Skin &skin, const FString &name, const FString &path)
{
	SP_UNUSED(skin);

	TSharedPtr<AtlasRegion> regionP = findRegion(path);
	if (!regionP) return nullptr;

	TSharedPtr<RegionAttachment> attachmentP = MakeShared <RegionAttachment>(name);

	attachmentP->SetAttachmentAtlasRegion(regionP->AsShared());

	return attachmentP;
}

TSharedPtr < MeshAttachment> AtlasAttachmentLoader::newMeshAttachment(Skin &skin, const FString &name, const FString &path)
{
	SP_UNUSED(skin);

	TSharedPtr<AtlasRegion> regionP = findRegion(path);
	if (!regionP) return NULL;


	TSharedPtr<MeshAttachment> attachmentP = MakeShared<MeshAttachment>(name);

	attachmentP->SetAttachmentAtlasRegion(regionP->AsShared());

	return attachmentP;
}

TSharedPtr < BoundingBoxAttachment> AtlasAttachmentLoader::newBoundingBoxAttachment(Skin &skin, const FString &name) {
	SP_UNUSED(skin);

	return  MakeShared<BoundingBoxAttachment>(name);

}

TSharedPtr < PathAttachment> AtlasAttachmentLoader::newPathAttachment(Skin &skin, const FString &name) {
	SP_UNUSED(skin);

	return  MakeShared<PathAttachment>(name);
}

TSharedPtr < PointAttachment> AtlasAttachmentLoader::newPointAttachment(Skin &skin, const FString &name) {
	SP_UNUSED(skin);
	
	return  MakeShared<PointAttachment>(name);
}

TSharedPtr < ClippingAttachment> AtlasAttachmentLoader::newClippingAttachment(Skin &skin, const FString &name) {
	SP_UNUSED(skin);

	return  MakeShared<ClippingAttachment>(name);
}

void AtlasAttachmentLoader::configureAttachment(TSharedPtr < Attachment> attachment)
{
	SP_UNUSED(attachment);
}

TSharedPtr<AtlasRegion> AtlasAttachmentLoader::findRegion(const FString &name) 
{
	return _atlas->findRegion(name);
}

}
