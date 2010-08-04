//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_IMAGE_CARD_LIST
#define HEADER_GUI_CONTROL_IMAGE_CARD_LIST

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/card_list.hpp>
#include <gui/control/filtered_card_list.hpp>

DECLARE_POINTER_TYPE(ImageField);

// ----------------------------------------------------------------------------- : ImageCardList

/// A card list that allows the shows thumbnails of card images
/** This card list also allows the list to be modified */
class ImageCardList : public CardListBase {
  public:
	~ImageCardList();
	ImageCardList(Window* parent, int id, long additional_style = 0);
  protected:
	virtual int  OnGetItemImage(long pos) const;
	virtual void onRebuild();
	virtual void onBeforeChangeSet();
	virtual bool allowModify() const { return true; }
  private:
	DECLARE_EVENT_TABLE();
	void onIdle(wxIdleEvent&);
	
	ImageFieldP image_field;			///< Field to use for card images
	mutable map<String,int> thumbnails;	///< image thumbnails, based on image_field
	
	ImageFieldP findImageField();
	
	friend class CardThumbnailRequest;
};

// ----------------------------------------------------------------------------- : FilteredImageCardList

class FilteredImageCardList : public ImageCardList {
  public:
	FilteredImageCardList(Window* parent, int id, long additional_style = 0);
	
	/// Change the filter to use, if null then don't use a filter
	void setFilter(const CardListFilterP& filter);
	
  protected:
	/// Get only the subset of the cards
	virtual void getItems(vector<VoidP>& out) const;
	virtual void onChangeSet();
	
  private:	
	CardListFilterP filter;	///< Filter with which this.cards is made
};

// ----------------------------------------------------------------------------- : EOF
#endif
