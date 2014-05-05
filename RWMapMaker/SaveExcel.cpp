/*
 * RWMapMaker is © IvimeyCom 2011-2014.
 * 
 * This file is part of RWMapMaker.
 * 
 * RWMapMaker is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 * 
 * RWMapMaker is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public License
 * along with RWMapMaker.  If not, see http://www.gnu.org/licenses/gpl.html.
 */

#include "stdafx.h"

#include <string>

#include "RWMapMaker.h"
#include "qxtlogger.h"
#include "ExcelFormat.h"
#include "map.h"
#include "Importer.h"

QRect fromQRectF(QRectF frect)
{
	QRect irect;
	irect.setCoords(frect.left(), frect.top(), frect.right(), frect.bottom()); 
	return irect;
}

void RWMapMaker::exportExcel(QString fileName)
{
	// create sheet 1 and get the associated BasicExcelWorksheet pointer
	ExcelFormat::BasicExcel xls;
	xls.New(1);
	ExcelFormat::BasicExcelWorksheet* sheet = xls.GetWorksheet(0);
	ExcelFormat::XLSFormatManager fmt_mgr(xls);

	// route title in cell 0,0 is bold arial text
	ExcelFormat::ExcelFont font_bold;
	font_bold.set_font_name(L"Arial");
	font_bold.set_weight(FW_BOLD); // 700

	ExcelFormat::CellFormat fmt_bold(fmt_mgr);
	fmt_bold.set_font(font_bold);

	QString ttl = rwimporter->getRouteName();
	ExcelFormat::BasicExcelCell* ttlcell = sheet->Cell(0,0);
	ttlcell->SetString(ttl.toUtf8().constData());
	ttlcell->SetFormat(fmt_bold);

	// tile cells are solid white bg with small arial text
	ExcelFormat::ExcelFont font_tile;
	font_tile.set_font_name(L"Arial");
	font_tile.set_height(140); // twips, 140 => 7pt
	font_tile.set_color_index(ExcelFormat::EGA_BLACK);

	ExcelFormat::CellFormat fmt_tile_track(fmt_mgr, font_tile);
	fmt_tile_track.set_background(ExcelFormat::EGA_WHITE,ExcelFormat::COLOR1_PAT_SOLID);
	fmt_tile_track.set_format_string(XLS_FORMAT_TEXT);
	fmt_tile_track.set_alignment(ExcelFormat::EXCEL_VALIGN_BOTTOM | ExcelFormat::EXCEL_HALIGN_CENTRED);
	fmt_tile_track.set_borderlines(ExcelFormat::EXCEL_LS_THIN, ExcelFormat::EXCEL_LS_THIN,
							 ExcelFormat::EXCEL_LS_THIN, ExcelFormat::EXCEL_LS_THIN,
							 ExcelFormat::EGA_BLACK, ExcelFormat::EGA_BLACK);

	ExcelFormat::CellFormat fmt_tile_terr(fmt_mgr, font_tile);
	fmt_tile_terr.set_background(ExcelFormat::EGA_GREEN,ExcelFormat::COLOR1_PAT_SOLID); // open dots
	fmt_tile_terr.set_format_string(XLS_FORMAT_TEXT);
	fmt_tile_terr.set_alignment(ExcelFormat::EXCEL_VALIGN_BOTTOM | ExcelFormat::EXCEL_HALIGN_CENTRED);
	fmt_tile_terr.set_borderlines(ExcelFormat::EXCEL_LS_THIN, ExcelFormat::EXCEL_LS_THIN,
							 ExcelFormat::EXCEL_LS_THIN, ExcelFormat::EXCEL_LS_THIN,
							 ExcelFormat::EGA_BLACK, ExcelFormat::EGA_BLACK);
	
	TileExtentMap *trackExtentMap = rwimporter->getTrackTileMap();
	TileExtentMap *terrainExtentMap = rwimporter->getTerrainTileMap();

	QRect overallRect = fromQRectF(trackExtentMap->getRegion());
	QRect terrainRect = fromQRectF(terrainExtentMap->getRegion());
	overallRect = overallRect.united(terrainRect);

	// GridType_Terrain:
    markTiles(overallRect, terrainExtentMap, (void*)sheet, (void*)&fmt_tile_terr);

    // GridType_Track:
    markTiles(overallRect, trackExtentMap, (void*)sheet, (void*)&fmt_tile_track);

	const int rH = 40*72;
	const int cW = 12*72;
	const int cols = sheet->GetTotalCols();
	const int rows = sheet->GetTotalRows();
	for (int x = 0; x <= cols; x++)
		sheet->SetColWidth(x, cW);

	for (int y = 0; y <= rows; y++)
		sheet->SetRowHeight(y, rH);

	xls.SaveAs(fileName.toUtf8().constData());
	xls.Close();
}

void RWMapMaker::markTiles(QRect overRect, TileExtentMap *extentMap, void* sheet_v, void* fmt_tile_v)
{
	ExcelFormat::BasicExcelWorksheet* sheet = (ExcelFormat::BasicExcelWorksheet*) sheet_v;
	ExcelFormat::CellFormat* fmt_tile = (ExcelFormat::CellFormat*)fmt_tile_v;

	const int headeroffset = 1;
	QImage *bitmap = extentMap->getImage();
	if (bitmap == NULL)
		return;

	// need to reverse cell Y values as they increase down, and map Y's increase up.
	const int cellsH = overRect.height();

    for (int x = overRect.left(); x <= overRect.right(); x += 1)
    {
        for (int y = overRect.top(); y <= overRect.bottom(); y += 1)
        {
            // extent can be pos or neg, but bitmap isn't: adjust to the centre.
            QPointF loc(x, y);
            QPointF bm_loc = MapWidget::adjustBitmapCentre(bitmap, loc);
            int v = bitmap->pixelIndex((int)bm_loc.x(), (int)bm_loc.y());
            if (v == INDEX_TILE)
            {
				// adjust "rect" coords to begin at 0,0
                QPoint cellRC;
				if (!rotateExcel)
					cellRC = QPoint(x, (cellsH - y) + headeroffset) - overRect.topLeft();
				else
					cellRC = QPoint((cellsH - y) + headeroffset, x) - overRect.topLeft();

                QString text = QString("%1 / %2").arg(x).arg(y);
				ExcelFormat::BasicExcelCell* cell = sheet->Cell(cellRC.y(), cellRC.x());

				cell->SetString(text.toUtf8().constData());
				cell->SetFormat(*fmt_tile);
            }
        }
    }
}
