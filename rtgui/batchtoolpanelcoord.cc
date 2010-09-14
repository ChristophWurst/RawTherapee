/*
 *  This file is part of RawTherapee.
 *
 *  Copyright (c) 2004-2010 Gabor Horvath <hgabor@rawtherapee.com>
 *
 *  RawTherapee is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  RawTherapee is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RawTherapee.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <multilangmgr.h>
#include <batchtoolpanelcoord.h>
#include <options.h>
#include <filepanel.h>
#include <procparamchangers.h>
#include <addsetids.h>

using namespace rtengine::procparams;

BatchToolPanelCoordinator::BatchToolPanelCoordinator (FilePanel* parent) : ToolPanelCoordinator(), parent(parent) {

	// remove exif panel and iptc panel
	std::vector<ToolPanel*>::iterator epi = std::find (toolPanels.begin(), toolPanels.end(), exifpanel);
	if (epi!=toolPanels.end())
		toolPanels.erase (epi);
	std::vector<ToolPanel*>::iterator ipi = std::find (toolPanels.begin(), toolPanels.end(), iptcpanel);
	if (ipi!=toolPanels.end())
		toolPanels.erase (ipi);
	toolPanelNotebook->remove_page (*metadataPanel);

    for (int i=0; i<toolPanels.size(); i++)
        toolPanels[i]->setBatchMode (true);
}

void BatchToolPanelCoordinator::selectionChanged (const std::vector<Thumbnail*>& selected) {

    if (selected!=this->selected) {
        closeSession ();
        this->selected = selected;
        selFileNames.clear ();
        for (int i=0; i<selected.size(); i++)
            selFileNames.push_back (selected[i]->getFileName ());
        initSession ();
    }
}

void BatchToolPanelCoordinator::closeSession (bool save) {

    pparamsEdited.set (false);        

    for (int i=0; i<selected.size(); i++)
        selected[i]->removeThumbnailListener (this);

    if (somethingChanged && save) {
        
        // read new values from the gui
        for (int i=0; i<toolPanels.size(); i++)
            toolPanels[i]->write (&pparams, &pparamsEdited);

        // combine with initial parameters and set
        ProcParams newParams;
        for (int i=0; i<selected.size(); i++) {
            newParams = initialPP[i];
            pparamsEdited.combine (newParams, pparams);
            selected[i]->setProcParams (newParams, BATCHEDITOR, true);
        }
    }
    for (int i=0; i<paramcListeners.size(); i++)
        paramcListeners[i]->clearParamChanges ();
}

void BatchToolPanelCoordinator::initSession () {

    somethingChanged = false;

    initialPP.resize (selected.size());
    for (int i=0; i<selected.size(); i++) {
        initialPP[i] = selected[i]->getProcParams ();
        selected[i]->applyAutoExp (initialPP[i]);
        selected[i]->addThumbnailListener (this);
    }

    pparamsEdited.initFrom (initialPP);

/*    curve->setAdjusterBehavior (false, false, false, false);
    whitebalance->setAdjusterBehavior (false, false);
    vignetting->setAdjusterBehavior (false);
    rotate->setAdjusterBehavior (false);
    distortion->setAdjusterBehavior (false);
    cacorrection->setAdjusterBehavior (false, false);
    colorshift->setAdjusterBehavior (false, false);
    colorboost->setAdjusterBehavior (false);
    lumadenoise->setAdjusterBehavior (false);
    sharpening->setAdjusterBehavior (false);
    shadowshighlights->setAdjusterBehavior (false, false, false);
*/
    crop->setDimensions (100000, 100000);

/*    if (selected.size()>0) {
        pparams = selected[0]->getProcParams ();
        for (int i=0; i<toolPanels.size(); i++) {
            toolPanels[i]->setDefaults (&pparams, &pparamsEdited);
            toolPanels[i]->read (&pparams, &pparamsEdited);
        }
        for (int i=0; i<paramcListeners.size(); i++)
            paramcListeners[i]->procParamsChanged (&pparams, rtengine::EvPhotoLoaded, "batch processing", &pparamsEdited);
    }
*/

    if (selected.size()>0) {

        pparams = selected[0]->getProcParams ();
        coarse->initBatchBehavior ();

        curve->setAdjusterBehavior (options.baBehav[ADDSET_TC_EXPCOMP], options.baBehav[ADDSET_TC_BRIGHTNESS], options.baBehav[ADDSET_TC_BLACKLEVEL], options.baBehav[ADDSET_TC_CONTRAST]);
        lcurve->setAdjusterBehavior (options.baBehav[ADDSET_LC_BRIGHTNESS], options.baBehav[ADDSET_LC_CONTRAST]);
        whitebalance->setAdjusterBehavior (options.baBehav[ADDSET_WB_TEMPERATURE], options.baBehav[ADDSET_WB_GREEN]);
        vignetting->setAdjusterBehavior (options.baBehav[ADDSET_VIGN_AMOUNT]);
        rotate->setAdjusterBehavior (options.baBehav[ADDSET_ROTATE_DEGREE]);
        distortion->setAdjusterBehavior (options.baBehav[ADDSET_DIST_AMOUNT]);
        perspective->setAdjusterBehavior (options.baBehav[ADDSET_PERSPECTIVE]);
        cacorrection->setAdjusterBehavior (options.baBehav[ADDSET_CA]);
        colorshift->setAdjusterBehavior (options.baBehav[ADDSET_CS_BLUEYELLOW], options.baBehav[ADDSET_CS_GREENMAGENTA]);
        colorboost->setAdjusterBehavior (options.baBehav[ADDSET_CBOOST_AMOUNT]);
        lumadenoise->setAdjusterBehavior (options.baBehav[ADDSET_LD_EDGETOLERANCE]);
        sharpening->setAdjusterBehavior (options.baBehav[ADDSET_SHARP_AMOUNT]);
        shadowshighlights->setAdjusterBehavior (options.baBehav[ADDSET_SH_HIGHLIGHTS], options.baBehav[ADDSET_SH_SHADOWS], options.baBehav[ADDSET_SH_LOCALCONTRAST]);
        
        if (options.baBehav[ADDSET_TC_EXPCOMP])  pparams.toneCurve.expcomp = 0;
        if (options.baBehav[ADDSET_TC_BRIGHTNESS])  pparams.toneCurve.brightness = 0;
        if (options.baBehav[ADDSET_TC_BLACKLEVEL])  pparams.toneCurve.black = 0;
        if (options.baBehav[ADDSET_TC_CONTRAST])  pparams.toneCurve.contrast = 0;

        if (options.baBehav[ADDSET_SH_HIGHLIGHTS])  pparams.sh.highlights = 0;
        if (options.baBehav[ADDSET_SH_SHADOWS])  pparams.sh.shadows = 0;
        if (options.baBehav[ADDSET_SH_LOCALCONTRAST])  pparams.sh.localcontrast = 0;

        if (options.baBehav[ADDSET_LC_BRIGHTNESS])  pparams.lumaCurve.brightness = 0;
        if (options.baBehav[ADDSET_LC_CONTRAST])  pparams.lumaCurve.contrast = 0;
        
        if (options.baBehav[ADDSET_SHARP_AMOUNT])  pparams.sharpening.amount = 0;
        if (options.baBehav[ADDSET_LD_EDGETOLERANCE])  pparams.lumaDenoise.edgetolerance = 0;

        if (options.baBehav[ADDSET_WB_TEMPERATURE])  pparams.wb.temperature = 0;
        if (options.baBehav[ADDSET_WB_GREEN])  pparams.wb.green = 0;

        if (options.baBehav[ADDSET_CBOOST_AMOUNT])  pparams.colorBoost.amount = 0;

        if (options.baBehav[ADDSET_CS_BLUEYELLOW])  pparams.colorShift.a = 0;
        if (options.baBehav[ADDSET_CS_GREENMAGENTA])  pparams.colorShift.b = 0;

        if (options.baBehav[ADDSET_ROTATE_DEGREE])  pparams.rotate.degree = 0;
        if (options.baBehav[ADDSET_DIST_AMOUNT])  pparams.distortion.amount = 0;
        if (options.baBehav[ADDSET_PERSPECTIVE])  pparams.perspective.horizontal = pparams.perspective.vertical = 0;
        if (options.baBehav[ADDSET_CA])  pparams.cacorrection.red = 0;
        if (options.baBehav[ADDSET_CA])  pparams.cacorrection.blue = 0;
        if (options.baBehav[ADDSET_VIGN_AMOUNT])  pparams.vignetting.amount = 0;

        for (int i=0; i<toolPanels.size(); i++) {
            toolPanels[i]->setDefaults (&pparams, &pparamsEdited);
            toolPanels[i]->read (&pparams, &pparamsEdited);
        }
        for (int i=0; i<paramcListeners.size(); i++)
	  paramcListeners[i]->procParamsChanged (&pparams, rtengine::EvPhotoLoaded, M("BATCH_PROCESSING"), &pparamsEdited);
    }
}

void BatchToolPanelCoordinator::panelChanged (rtengine::ProcEvent event, const Glib::ustring& descr) {

    if (selected.size()==0)
        return;

    somethingChanged = true;

    pparamsEdited.set (false);        
    // read new values from the gui
    for (int i=0; i<toolPanels.size(); i++)
        toolPanels[i]->write (&pparams, &pparamsEdited);

    if (event==rtengine::EvAutoExp || event==rtengine::EvClip) 
        for (int i=0; i<selected.size(); i++) {
            initialPP[i].toneCurve.autoexp = pparams.toneCurve.autoexp;
            initialPP[i].toneCurve.clip = pparams.toneCurve.clip;
            selected[i]->applyAutoExp (initialPP[i]);
        }

    // combine with initial parameters and set
    ProcParams newParams;
    for (int i=0; i<selected.size(); i++) {
        newParams = initialPP[i];
        pparamsEdited.combine (newParams, pparams);
        selected[i]->setProcParams (newParams, BATCHEDITOR, false);
    }

    for (int i=0; i<paramcListeners.size(); i++)
        paramcListeners[i]->procParamsChanged (&pparams, event, descr, &pparamsEdited);
}

void BatchToolPanelCoordinator::getAutoWB (double& temp, double& green) {

    if (selected.size()>0)
        selected[0]->getAutoWB (temp, green);       
}

void BatchToolPanelCoordinator::getCamWB (double& temp, double& green) {
    
    if (selected.size()>0)
        selected[0]->getCamWB (temp, green);
}    

void BatchToolPanelCoordinator::optionsChanged () {

    closeSession ();
    initSession ();
}

void BatchToolPanelCoordinator::procParamsChanged (Thumbnail* thm, int whoChangedIt) {

    if (whoChangedIt!=BATCHEDITOR) {
        closeSession (false);
        initSession ();
    }
}

void BatchToolPanelCoordinator::profileChange  (const ProcParams *nparams, rtengine::ProcEvent event, const Glib::ustring& descr, const ParamsEdited* paramsEdited) {

    pparams = *nparams;
    if (paramsEdited)
        pparamsEdited = *paramsEdited;

    for (int i=0; i<toolPanels.size(); i++) 
        toolPanels[i]->read (&pparams, &pparamsEdited);

    somethingChanged = true;

    // read new values from the gui
    for (int i=0; i<toolPanels.size(); i++)
        toolPanels[i]->write (&pparams, &pparamsEdited);

    // combine with initial parameters and set
    ProcParams newParams;
    for (int i=0; i<selected.size(); i++) {
        newParams = initialPP[i];
        pparamsEdited.combine (newParams, pparams);
        selected[i]->setProcParams (newParams, BATCHEDITOR, false);
    }

    for (int i=0; i<paramcListeners.size(); i++)
        paramcListeners[i]->procParamsChanged (&pparams, event, descr, &pparamsEdited);
}

void BatchToolPanelCoordinator::cropSelectionReady () {

  toolBar->setTool (TMHand);
}

CropGUIListener* BatchToolPanelCoordinator::startCropEditing (Thumbnail* thm) {
    
    if (thm) {
        int w, h;
        thm->getFinalSize (thm->getProcParams (), w, h);
        crop->setDimensions (w, h);
    }
    return crop;
}

void BatchToolPanelCoordinator::rotateSelectionReady (double rotate_deg, Thumbnail* thm) {

  toolBar->setTool (TMHand);
  if (rotate_deg!=0.0)
      rotate->straighten (rotate_deg);
}

void BatchToolPanelCoordinator::spotWBselected (int x, int y, Thumbnail* thm) {

//    toolBar->setTool (TOOL_HAND);
    if (x>0 && y>0 && thm) {
        for (int i=0; i<selected.size(); i++)
            if (selected[i]==thm) {
                double temp;
                double green;
                thm->getSpotWB (x, y, whitebalance->getSize(), temp, green);
                double otemp = initialPP[i].wb.temperature;
                double ogreen = initialPP[i].wb.green;
                if (options.baBehav[12])
                    temp = temp - otemp;
                if (options.baBehav[13])
                    green = green - ogreen;
                whitebalance->setWB (temp, green);
            }
    }
}
