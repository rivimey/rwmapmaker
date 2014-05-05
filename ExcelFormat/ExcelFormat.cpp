/*
	ExcelFormat.cpp

	Copyright (c) 2009, 2011 Martin Fuchs <martin-fuchs@gmx.net>

	License: CPOL

	THE WORK (AS DEFINED BELOW) IS PROVIDED UNDER THE TERMS OF THIS CODE PROJECT OPEN LICENSE ("LICENSE").
	THE WORK IS PROTECTED BY COPYRIGHT AND/OR OTHER APPLICABLE LAW. ANY USE OF THE WORK OTHER THAN AS
	AUTHORIZED UNDER THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
	BY EXERCISING ANY RIGHTS TO THE WORK PROVIDED HEREIN, YOU ACCEPT AND AGREE TO BE BOUND BY THE TERMS
	OF THIS LICENSE. THE AUTHOR GRANTS YOU THE RIGHTS CONTAINED HEREIN IN CONSIDERATION OF YOUR ACCEPTANCE
	OF SUCH TERMS AND CONDITIONS. IF YOU DO NOT AGREE TO ACCEPT AND BE BOUND BY THE TERMS OF THIS LICENSE,
	YOU CANNOT MAKE ANY USE OF THE WORK.
*/

#include "ExcelFormat.h"

using namespace ExcelFormat;
using namespace std;

ExcelFont::ExcelFont()
	:	_height(200),
	_options(0),
	_color_index(0x7FFF),
	_weight(400), // FW_NORMAL
	_escapement_type(EXCEL_ESCAPEMENT_NONE),
	_underline_type(EXCEL_UNDERLINE_NONE),
	_font_family(EXCEL_FONT_FAMILY_DONTCARE),
	_character_set(0), // ANSI_CHARSET
	_name(L"Arial")
{
}

ExcelFont::ExcelFont(const Workbook::Font& font)
	:	_height(font.height_),
	_options(font.options_),
	_color_index(font.colourIndex_),
	_weight(font.weight_),
	_escapement_type(font.escapementType_),
	_underline_type(font.underlineType_),
	_font_family(font.family_),
	_character_set(font.characterSet_),
	_name(wstringFromSmallString(font.name_))
{
}

ExcelFont& ExcelFont::set_height(short height)
{
	_height = height;
	return *this;
}

ExcelFont& ExcelFont::set_weight(short weight)
{
	_weight = weight;
	return *this;
}

ExcelFont& ExcelFont::set_italic(bool italic)
{
	_options = (_options&~EXCEL_FONT_ITALIC) | (italic? EXCEL_FONT_ITALIC: 0);
	return *this;
}

ExcelFont& ExcelFont::set_color_index(short color_idx)
{
	_color_index = color_idx;
	return *this;
}

ExcelFont& ExcelFont::set_underline_type(char underline_type)
{
	_underline_type = underline_type;
	return *this;
}

ExcelFont& ExcelFont::set_escapement(short escapement_type)
{
	_escapement_type = escapement_type;
	return *this;
}

ExcelFont& ExcelFont::set_font_name(const wchar_t* name)
{
	_name = name;
	return *this;
}

bool ExcelFont::matches(const Workbook::Font& font) const
{
	if (wstringFromSmallString(font.name_) != _name)
		return false;

	if (font.height_ != _height)
		return false;

	if (font.weight_ != _weight)
		return false;

	if (font.options_ != _options)
		return false;

	if (font.colourIndex_ != _color_index)
		return false;

	if (font.escapementType_ != _escapement_type)
		return false;

	if (font.underlineType_ != _underline_type)
		return false;

	if (font.family_ != _font_family)
		return false;

	if (font.characterSet_ != _character_set)
		return false;

	return true;
}


CellFormat::CellFormat(XLSFormatManager& mgr)
	:	_mgr(mgr)
{
	init();
}

CellFormat::CellFormat(XLSFormatManager& mgr, const ExcelFont& font)
	:	_mgr(mgr)
{
	init();
	set_font(font);
}

CellFormat::CellFormat(XLSFormatManager& mgr, const Workbook::XF& xf, short xf_idx)
	:	_mgr(mgr)
{
	init(xf, xf_idx);
}

CellFormat::CellFormat(XLSFormatManager& mgr, const BasicExcelCell* cell)
	:	_mgr(mgr)
{
	short xf_idx = cell->GetXFormatIdx();
	init(mgr.get_XF(xf_idx), xf_idx);
}

	 // set borderlines using EXCEL_LS enumeration constants and colour indices
CellFormat& CellFormat::set_borderlines(
	EXCEL_LS left, EXCEL_LS right, EXCEL_LS top, EXCEL_LS bottom,
	unsigned idxLclr, unsigned idxRclr)
{
	_borderlines = MAKE_BORDERSTYLE(left, right, top, bottom, idxLclr, idxRclr);
	flush();
	return *this;
}


CellFormat& CellFormat::set_borderlines(
	EXCEL_LS left, EXCEL_LS right, EXCEL_LS top, EXCEL_LS bottom,
	unsigned idxLclr, unsigned idxRclr, unsigned idxTclr, unsigned idxBclr,
	EXCEL_LS diag,		// diagonal line style
	COLOR1_PAT pattern	// fill pattern
)
{
	_borderlines = MAKE_BORDERSTYLE(left, right, top, bottom, idxLclr, idxRclr);
	_color1 = MAKE_COLOR1(idxTclr, idxBclr, diag, pattern);
	flush();
	return *this;
}

CellFormat& CellFormat::set_format_string(const wstring& fmt_str)
{
	_fmt_idx = _mgr.get_format_idx(fmt_str);
	flush();
	return *this;
}

CellFormat& CellFormat::set_font(const ExcelFont& font)
{
	_font_idx = _mgr.get_font_idx(font);
	flush();
	return *this;
}

CellFormat& CellFormat::set_color1(unsigned color)
{
	_color1 = color;
	flush();
	return *this;
}

CellFormat& CellFormat::set_color2(unsigned short color)
{
	_color2 = color;
	flush();
	return *this;
}

CellFormat& CellFormat::set_alignment(char alignment)
{
	_alignment = alignment;
	flush();
	return *this;
}

CellFormat& CellFormat::set_rotation(char rotation)
{
	_rotation = rotation;
	flush();
	return *this;
}

CellFormat& CellFormat::set_text_props(char text_props)
{
	_text_props = text_props;
	flush();
	return *this;
}

CellFormat& CellFormat::set_borderlines(int borderlines)
{
	_borderlines = borderlines;
	flush();
	return *this;
}

short CellFormat::get_xf_idx() const
{
	if (_xf_idx == -1)
		_xf_idx = _mgr.get_xf_idx(*this);

	return _xf_idx;
}

bool CellFormat::matches(const Workbook::XF& xf) const
{
	if (xf.fontRecordIndex_ != _font_idx)
		return false;

	if (xf.formatRecordIndex_ != _fmt_idx)
		return false;

	if (xf.alignment_ != _alignment)
		return false;

	if (xf.rotation_ != _rotation)
		return false;

	if (xf.textProperties_ != _text_props)
		return false;

	if (xf.borderLines_ != _borderlines)
		return false;

	if (xf.colour1_ != _color1)
		return false;

	if (xf.colour2_ != _color2)
		return false;

	return true;
}

void CellFormat::get_xf(Workbook::XF& xf) const
{
	xf.fontRecordIndex_ = _font_idx;
	xf.formatRecordIndex_ = _fmt_idx;
	xf.alignment_ = _alignment;
	xf.rotation_= _rotation;
	xf.textProperties_= _text_props;
	xf.borderLines_= _borderlines;
	xf.colour1_ = _color1;
	xf.colour2_ = _color2;
}

void CellFormat::init()
{
	_font_idx = 0;
	_fmt_idx = 0;
	_alignment = EXCEL_VALIGN_BOTTOM;
	_rotation = 0;
	_text_props = MAKE_TEXT_PROPS(0, EXCEL_TEXT_PROP_DEFAULT);
	_borderlines = 0;
	_color1 = 0;
	_color2 = MAKE_COLOR2(64, 65); // 0x20C0

	_xf_idx = 0;
}

void CellFormat::init(const Workbook::XF& xf, short xf_idx)
{
	_font_idx = xf.fontRecordIndex_;
	_fmt_idx = xf.formatRecordIndex_;
	_alignment = xf.alignment_;
	_rotation = xf.rotation_;
	_text_props = xf.textProperties_;
	_borderlines = xf.borderLines_;
	_color1 = xf.colour1_;
	_color2 = xf.colour2_;
	_xf_idx = xf_idx;
}



XLSFormatManager::XLSFormatManager(BasicExcel& xls)
 :	_xls(xls),
	_next_fmt_idx(FIRST_USER_FORMAT_IDX)	// above last reserved format index 163
{
	 // initialize predefined formats
	_formats[0] = XLS_FORMAT_GENERAL;		// "General"								// General
	_formats[1] = XLS_FORMAT_INTEGER;		// "0"										// Decimal
	_formats[2] = XLS_FORMAT_DECIMAL;		// "0.00"									// Decimal
	_formats[3] = L"#,##0";																// Decimal
	_formats[4] = L"#,##0.00";															// Decimal
	_formats[5] = L"\"$\"#,##0_);(\"$\"#,##0)";											// Currency
	_formats[6] = L"\"$\"#,##0_);[Red](\"$\"#,##0)";									// Currency
	_formats[7] = L"\"$\"#,##0.00_);(\"$\"#,##0.00)";									// Currency
	_formats[8] = L"\"$\"#,##0.00_);[Red](\"$\"#,##0.00)";								// Currency
	_formats[9] = XLS_FORMAT_PERCENT;		// "0%"										// Percent
	_formats[10] = L"0.00%";															// Percent
	_formats[11] = L"0.00E+00";															// Scientific
	_formats[12] = L"# ?/?";															// Fraction
	_formats[13] = L"# ?\?/?\?";															// Fraction
	_formats[14] = XLS_FORMAT_DATE;			// "M/D/YY"									// Date
	_formats[15] = L"D-MMM-YY";															// Date
	_formats[16] = L"D-MMM";															// Date
	_formats[17] = L"MMM-YY";															// Date
	_formats[18] = L"h:mm AM/PM";														// Time
	_formats[19] = L"h:mm:ss AM/PM";													// Time
	_formats[20] = L"h:mm";																// Time
	_formats[21] = XLS_FORMAT_TIME;			// "h:mm:ss"								// Time
	_formats[22] = XLS_FORMAT_DATETIME;		// "M/D/YY h:mm"							// Date/Time
	_formats[37] = L"_(#,##0_);(#,##0)";												// Account.
	_formats[38] = L"_(#,##0_);[Red](#,##0)";											// Account.
	_formats[39] = L"_(#,##0.00_);(#,##0.00)";											// Account.
	_formats[40] = L"_(#,##0.00_);[Red](#,##0.00)";										// Account.
	_formats[41] = L"_(* #,##0_);_(* (#,##0);_(* \"-\"_);_(@_)";						// Currency
	_formats[42] = L"_($* #,##0_);_($* (#,##0);_($* \"-\"_);_(@_)";						// Currency
	_formats[43] = L"_(* #,##0.00_);_(* (#,##0.00);_(* \"-\"??_);_(@_)";				// Currency
	_formats[44] = L"_($* #,##0.00_);_($* (#,##0.00);_($* \"-\"??_);_(@_)";				// Currency
	_formats[45] = L"mm:ss";															// Time
	_formats[46] = L"[h]:mm:ss";														// Time
	_formats[47] = L"mm:ss.0";															// Time
	_formats[48] = L"##0.0E+0";															// Scientific
	_formats[49] = XLS_FORMAT_TEXT;			// "@"										// Text

	 // overwrite formats from workbook
	size_t maxFormats = xls.workbook_.formats_.size();
	for(size_t i=0; i<maxFormats; ++i) {
		short idx = xls.workbook_.formats_[i].index_;

		_formats[idx] = wstringFromLargeString(xls.workbook_.formats_[i].fmtstring_);

		 // adjust index for the next user defined format
		if (idx >= _next_fmt_idx)
			_next_fmt_idx = idx + 1;
	}

	 // create reverse format map
	for(FormatMap::const_iterator it=_formats.begin(); it!=_formats.end(); ++it)
		_formats_rev[it->second] = it->first;
}


short XLSFormatManager::get_font_idx(const ExcelFont& font)
{
	short i = 0;
	for(vector<Workbook::Font>::const_iterator it=_xls.workbook_.fonts_.begin(); it!=_xls.workbook_.fonts_.end(); ++it,++i)
		if (font.matches(*it))
			return i;

	short font_idx = (short) _xls.workbook_.fonts_.size();
	_xls.workbook_.fonts_.push_back(Workbook::Font());
	Workbook::Font& new_font = _xls.workbook_.fonts_[font_idx];

	new_font.height_ = font.get_height();
	new_font.options_ = font.get_font_options();
	new_font.colourIndex_ = font.get_color_idx();
	new_font.weight_ = font.get_weight();
	new_font.escapementType_ = font.get_escapement();
	new_font.underlineType_ = font.get_underline_type();
	new_font.family_ = font.get_font_family();
	new_font.characterSet_ = font.get_character_set();

	new_font.name_ = font.get_font_name().c_str();

	 // The font with index 4 is omitted in all BIFF versions. This means the first four fonts have zero-based indexes,
	 // and the fifth font and all following fonts are referenced with one-based indexes.
	if (font_idx >= 4)
		++font_idx;

	return font_idx;
}

const Workbook::Font& XLSFormatManager::get_font(const CellFormat& fmt) const
{
	size_t font_idx = fmt.get_font_idx();

	if (font_idx > 4)
		--font_idx;

	if (font_idx < _xls.workbook_.fonts_.size())
		return _xls.workbook_.fonts_[font_idx];
	else {
//		assert(0); // font_idx out of range
		return _xls.workbook_.fonts_[0];
	}
}

wstring XLSFormatManager::get_format_string(const CellFormat& fmt) const
{
	short fmt_idx = fmt.get_fmt_idx();

	FormatMap::const_iterator found = _formats.find(fmt_idx);

	if (found != _formats.end())
		return found->second;
	else
		return XLS_FORMAT_GENERAL;
}

short XLSFormatManager::get_format_idx(const wstring& fmt_str)
{
	FormatRevMap::const_iterator found = _formats_rev.find(fmt_str);

	if (found != _formats_rev.end())
		return found->second;

	 // register a new format string
	short fmt_idx = _next_fmt_idx++;

	_formats[fmt_idx] = fmt_str;
	_formats_rev[fmt_str] = fmt_idx;

	_xls.workbook_.formats_.push_back(Workbook::Format());
	Workbook::Format& format = _xls.workbook_.formats_.back();

	format.index_ = fmt_idx;
	format.fmtstring_ = fmt_str.c_str();

	return fmt_idx;
}

short XLSFormatManager::get_xf_idx(const CellFormat& fmt)
{
	short i = 0;
	for(vector<Workbook::XF>::const_iterator it=_xls.workbook_.XFs_.begin(); it!=_xls.workbook_.XFs_.end(); ++it,++i)
		if (fmt.matches(*it))
			return i;

	 // create a new XF
	short xf_idx = (short) _xls.workbook_.XFs_.size();
	_xls.workbook_.XFs_.push_back(Workbook::XF());
	Workbook::XF& xf = _xls.workbook_.XFs_[xf_idx];

	fmt.get_xf(xf);

	xf.protectionType_ = 0 | (15 << 3);		// cell not locked, formula not hidden, type=Cell XF, parent style 15 (default)
//	xf.usedAttributes_ = (unsigned char)(0x3F << 2);	// XF_USED_ATTRIB: use attributes from parent style: font (0x02), ...

	return xf_idx;
}
