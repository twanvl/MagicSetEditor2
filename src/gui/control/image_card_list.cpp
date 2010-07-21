//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/image_card_list.hpp>
#include <gui/thumbnail_thread.hpp>
#include <data/field/image.hpp>
#include <data/game.hpp>
#include <data/card.hpp>
#include <gfx/gfx.hpp>
#include <wx/imaglist.h>

DECLARE_TYPEOF_COLLECTION(FieldP);

// ----------------------------------------------------------------------------- : ImageCardList

ImageCardList::ImageCardList(Window* parent, int id, long additional_style)
	: CardListBase(parent, id, additional_style)
{}

ImageCardList::~ImageCardList() {
	thumbnail_thread.abort(this);
}
void ImageCardList::onRebuild() {
	image_field = findImageField();
}
void ImageCardList::onBeforeChangeSet() {
	CardListBase::onBeforeChangeSet();
	// remove all but the first two (sort asc/desc) images from image list
	wxImageList* il = GetImageList(wxIMAGE_LIST_SMALL);
	while (il && il->GetImageCount() > 2) {
		il->Remove(2);
	}
	thumbnail_thread.abort(this);
	thumbnails.clear();
}

ImageFieldP ImageCardList::findImageField() {
	FOR_EACH(f, set->game->card_fields) {
		ImageFieldP imgf = dynamic_pointer_cast<ImageField>(f);
		if (imgf) return imgf;
	}
	return ImageFieldP();
}

/// A request for a thumbnail of a card image
class CardThumbnailRequest : public ThumbnailRequest {
  public:
	CardThumbnailRequest(ImageCardList* parent, const String& filename)
		: ThumbnailRequest(
			parent,
			_("card") + parent->set->absoluteFilename() + _("-") + filename,
			wxDateTime::Now())	// TODO: Find mofication time of card image
		, filename(filename)
	{}
	virtual Image generate() {
		try {
			ImageCardList* parent = (ImageCardList*)owner;
			Image image;
			if (image.LoadFile(*parent->set->openIn(filename))) {
				// two step anti aliased resampling
				image.Rescale(36, 28); // step 1: no anti aliassing
				return resample(image, 18, 14); // step 2: with anti aliassing
			} else {
				return Image();
			}
		} catch (...) {
			return Image();
		}
	}
	virtual void store(const Image& img) {
		// add finished bitmap to the imagelist
		ImageCardList* parent = (ImageCardList*)owner;
		if (img.Ok()) {
			wxImageList* il = parent->GetImageList(wxIMAGE_LIST_SMALL);
			int id = il->Add(wxBitmap(img));
			parent->thumbnails.insert(make_pair(filename, id));
			parent->Refresh(false);
		}
	}

	virtual bool threadSafe() const {return true;}
  private:
	String filename;
};

int ImageCardList::OnGetItemImage(long pos) const {
	if (image_field) {
		// Image = thumbnail of first image field of card
		ImageValue& val = static_cast<ImageValue&>(*getCard(pos)->data[image_field]);
		if (!val.filename) return -1; // no image
		// is there already a thumbnail?
		map<String,int>::const_iterator it = thumbnails.find(val.filename);
		if (it != thumbnails.end()) {
			return it->second;
		} else {
			// request a thumbnail
			thumbnail_thread.request(intrusive(new CardThumbnailRequest(const_cast<ImageCardList*>(this), val.filename)));
		}
	}
	return -1;
}

void ImageCardList::onIdle(wxIdleEvent&) {
	thumbnail_thread.done(this);
}


BEGIN_EVENT_TABLE(ImageCardList, CardListBase)
	EVT_IDLE	   (ImageCardList::onIdle)
END_EVENT_TABLE  ()
