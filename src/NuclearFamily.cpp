/////////////////////////////////////////////////////////
//
// This file is part of the MADELINE 2 program 
// written by Edward H. Trager and Ritu Khanna
// Copyright (c) 2005 by the
// Regents of the University of Michigan.
// All Rights Reserved.
// 
// The latest version of this program is available from:
// 
//   http://eyegene.ophthy.med.umich.edu/madeline/
//   
// Released under the GNU General Public License.
// A copy of the GPL is included in the distribution
// package of this software, or see:
// 
//   http://www.gnu.org/copyleft/
//   
// ... for licensing details.
// 
/////////////////////////////////////////////////////////

//
// NuclearFamily.cpp
//

#include "Individual.h"
#include "NuclearFamily.h"
#include "DrawingCanvas.h"
#include "DrawingMetrics.h"
#include "Number.h"

#include <deque>
#include <map>
#include <algorithm>

////////////////////////////////////////
//
// PRIVATE METHODS
//
////////////////////////////////////////

//
// _orderTwins:
//
void NuclearFamily::_orderTwins(Individual* pivotIndividual,std::deque<Individual*>& srcLoopIndividuals, std::deque<Individual*>& dstLoopIndividuals,const std::deque<unsigned>& indices, bool front ){
	
	std::deque<Individual*>::iterator dit;
	
	/*std::cout << "###DEBUG" << std::endl;
	std::cout << "DST" << std::endl;
	for(unsigned i=0;i<dstLoopIndividuals.size();i++) std::cout << dstLoopIndividuals[i]->getId() << std::endl;
	std::cout << "SRC" << std::endl;
	for(unsigned i=0;i<srcLoopIndividuals.size();i++) std::cout << srcLoopIndividuals[i]->getId() << std::endl;
	std::cout << "IND" << std::endl;
	for(unsigned i=0;i<indices.size();i++) std::cout << indices[i] << std::endl;
	std::cout << "pivot" << pivotIndividual->getId() << std::endl;
	*/
	
	// If the pivotIndividual exists in the dst, assign a pointer to the pivotIndividual
	unsigned insertStartIndex=0,insertEndIndex=0;
	bool hasPivotIndividual=false;
	dit = find(dstLoopIndividuals.begin(),dstLoopIndividuals.end(),pivotIndividual);
	if(dit != dstLoopIndividuals.end()){
		for(unsigned k=0;k<dstLoopIndividuals.size();k++)
		if(dstLoopIndividuals[k]->getId() == pivotIndividual->getId()) insertStartIndex = k;
		hasPivotIndividual=true;
	}
	
	// Insert the twins in src before the pivotIndividual pointer
	Individual* dtemp;
	
	std::deque<Individual*>::iterator ddit;
	unsigned cnt=0;
	while(cnt < indices.size()){
		dtemp = srcLoopIndividuals[indices[cnt]];
		if(hasPivotIndividual){ dstLoopIndividuals.insert(dit,dtemp); insertEndIndex=dstLoopIndividuals.size();}
		else if(front){ dstLoopIndividuals.push_front(dtemp); insertStartIndex=1; insertEndIndex=dstLoopIndividuals.size(); }
		else { dstLoopIndividuals.push_back(dtemp); insertStartIndex = 0;insertEndIndex = dstLoopIndividuals.size()-1; }
		//for(unsigned k=insertStartIndex;k<insertEndIndex;k++){
		//	if(dstLoopIndividuals[k]->getId() == dtemp->getId()) 
		//		std::cout << "DEBUG: THERE is a DUPLICATE ENTRY in the deque. Please delete it. " << dtemp->getId() << std::endl;
		//}
		cnt++;
	}
	
	// Delete the twin individuals from the src:
	std::deque<Individual*> temp;
	std::deque<unsigned>::const_iterator uit;
	
	for(unsigned j=0;j<srcLoopIndividuals.size();j++){
		uit = find(indices.begin(),indices.end(),j);
		if(uit != indices.end()) ;
		else temp.push_back(srcLoopIndividuals[j]);
	}
	srcLoopIndividuals.swap(temp);
	
	
}

//
// _rearrangeIndividualsBasedOnTwins:
//
void NuclearFamily::_rearrangeIndividualsBasedOnTwins(const bool consanguinousLoop,std::deque<Individual*>& initial,std::deque<Individual*>& leftLoopIndividuals,std::deque<Individual*>& rightLoopIndividuals){
	
	
	unsigned twinGroups = 0;
	
	// For twin cases the queues have to be rearranged to allow for all the individuals in a twin group to be adjacent
	std::vector<std::string> twinMarkers;
	while(twinGroups < _twinGroupCount){
		
		// First check the left queue
		unsigned leftCount=0,rightCount=0,initialCount=0;
		std::deque<Individual*>::iterator dit;
		
		// Store the indices of where the twins are in the loop deques
		std::deque<unsigned> leftTwins;
		std::deque<unsigned> rightTwins;
		std::deque<unsigned> initialTwins;
		std::string marker;
		
		Individual* pivotIndividual,*pivotRightIndividual;
		unsigned leftStartIndex=0,rightStartIndex=0;
		
		// First mark all the twins in the left Q
		for(unsigned i=0;i<leftLoopIndividuals.size();i++){
			marker = leftLoopIndividuals[i]->getTwinMarker().getMarker();
			if(marker != "."){
				std::vector<std::string>::iterator vit = find(twinMarkers.begin(),twinMarkers.end(),marker);
				if(vit != twinMarkers.end()) continue; 
				twinMarkers.push_back(marker);
				leftCount++;
				leftStartIndex = i;
				// Check how many more twins are in the different qs
				unsigned pivotIndex=i;
				for(unsigned j=i+1;j<leftLoopIndividuals.size();j++){
					if(leftLoopIndividuals[j]->getTwinMarker().getMarker() == marker){
						if(pivotIndex+1 == j) pivotIndex = j;
						leftCount++;
						leftTwins.push_back(j);
					}
				}
				if(pivotIndex == 0) pivotIndividual = new Individual();
				else pivotIndividual = leftLoopIndividuals[pivotIndex];
				for(unsigned j=0;j<rightLoopIndividuals.size();j++){
					if(rightLoopIndividuals[j]->getTwinMarker().getMarker() == marker && rightLoopIndividuals[j]->getId() != leftLoopIndividuals[leftStartIndex]->getId()){
						if(j+1 == rightLoopIndividuals.size()) pivotRightIndividual = new Individual();
						else
						pivotRightIndividual = rightLoopIndividuals[j+1];
						rightStartIndex = j;
						rightTwins.push_back(j);
						rightCount++;
					}
				}
				for(unsigned j=0;j<initial.size();j++){
					if(initial[j]->getTwinMarker().getMarker() == marker){
						initialCount++;
						initialTwins.push_back(j);
					}
				}
				break;
			}
		}
		// There were no twins in the left Q; check the right Q:
		if(leftCount == 0 && rightCount == 0)
		for(unsigned i=0;i<rightLoopIndividuals.size();i++){
			marker = rightLoopIndividuals[i]->getTwinMarker().getMarker();
			if(marker != "."){
				std::vector<std::string>::iterator vit = find(twinMarkers.begin(),twinMarkers.end(),marker);
				if(vit != twinMarkers.end()) continue;
				twinMarkers.push_back(marker);
				rightCount++;
				rightStartIndex = i;
				if(i+1 == rightLoopIndividuals.size()) pivotIndividual = new Individual();
				else
				pivotIndividual = rightLoopIndividuals[i+1];
				
				
				// Check how many more twins are in the different qs
				for(unsigned j=i+1;j<rightLoopIndividuals.size();j++){
					if(rightLoopIndividuals[j]->getTwinMarker().getMarker() == marker){
						rightCount++;
						rightTwins.push_back(j);
						if(j+1 < rightLoopIndividuals.size()) pivotIndividual = rightLoopIndividuals[j+1];
						else pivotIndividual = new Individual();
					}
				}
				for(unsigned j=0;j<initial.size();j++){
					if(initial[j]->getTwinMarker().getMarker() == marker){
						initialCount++;
						initialTwins.push_back(j);
					}
				}
				break;
			}
		}
		// Reorder the twins in different Qs
		if(rightCount == 0 && leftCount > 0){
			// Need to manipulate left and initial
			// Push all the twins in initial to the left q
			_orderTwins(pivotIndividual,initial,leftLoopIndividuals,initialTwins,true);
			// Push all the twins in the left q adjacent to the pivot Individual
			if(leftTwins.size()) _orderTwins(pivotIndividual,leftLoopIndividuals,leftLoopIndividuals,leftTwins);  
		}else if(leftCount == 0 && rightCount > 0){
			// Need to manipulate right and initial
			// Push all the twins in the right q adjacent to the pivot Individual
			if(rightTwins.size()) _orderTwins(pivotIndividual,rightLoopIndividuals,rightLoopIndividuals,rightTwins); 
			// Push all the twins in initial to the right q
			_orderTwins(pivotIndividual,initial,rightLoopIndividuals,initialTwins);
			
		}else if(leftCount > 0 && rightCount > 0){
			// Need to manipulate both the Qs
			// Check to see which side has more # of twins
			if(leftCount == rightCount){
				// check to see which side has higher number
				if(leftLoopIndividuals[leftStartIndex]->getLeftFlag(consanguinousLoop) == rightLoopIndividuals[rightTwins.front()]->getRightFlag(consanguinousLoop)){
					if(initialTwins.size()) _orderTwins(pivotIndividual,initial,leftLoopIndividuals,initialTwins,true);
					_orderTwins(pivotIndividual,rightLoopIndividuals,leftLoopIndividuals,rightTwins);
				}else
				if(leftTwins.size() && leftLoopIndividuals[leftTwins.front()]->getLeftFlag(consanguinousLoop) > rightLoopIndividuals[rightTwins.front()]->getRightFlag(consanguinousLoop)){
					// Push everything to the left
					if(initialTwins.size()) _orderTwins(pivotIndividual,initial,leftLoopIndividuals,initialTwins,true);
					_orderTwins(pivotIndividual,rightLoopIndividuals,leftLoopIndividuals,rightTwins,true);
				}else{
					// Push everything to the right 
					leftTwins.push_front(leftStartIndex);
					_orderTwins(pivotRightIndividual,leftLoopIndividuals,rightLoopIndividuals,leftTwins);
					if(initialTwins.size()) _orderTwins(pivotRightIndividual,initial,rightLoopIndividuals,initialTwins);
				}
			}else if(leftCount > rightCount){
					// Push everything to the left
					if(initialTwins.size()) _orderTwins(pivotIndividual,initial,leftLoopIndividuals,initialTwins,true);
					_orderTwins(pivotIndividual,rightLoopIndividuals,leftLoopIndividuals,rightTwins,true);
			}else{
				// Push everything to the right 
				leftTwins.push_front(leftStartIndex);
				_orderTwins(pivotRightIndividual,leftLoopIndividuals,rightLoopIndividuals,leftTwins);
				if(initialTwins.size()) _orderTwins(pivotRightIndividual,initial,rightLoopIndividuals,initialTwins);
			}
		}
		
		twinGroups++;
	}
	
}

// 
// _isMaleWithLoopFlags: All males with consanguinous or external connections do not have the NF drawn with them; the NF is drawn with the spouse
//
inline bool NuclearFamily::_isMaleWithLoopFlags(Individual* individual,unsigned nuclearFamilyIndex){
	
	if(individual->getGender().getEnum() == Gender::MALE && (individual->getNuclearFamily(nuclearFamilyIndex)->isConsanguinous() || individual->getNuclearFamily(nuclearFamilyIndex)->hasExternalConnection()))
		return true;
	return false;
	
}

//
// _drawTwinConnectors:
//
void NuclearFamily::_drawTwinConnectors(DrawingCanvas& dc,bool classicalOrder){
	
	//
	// Determine which ordering of children to use
	//
	std::vector<Individual*> children;
	if(classicalOrder) children.assign(_childrenInClassicalOrder.begin(),_childrenInClassicalOrder.end());
	else children.assign(_sortedChildren.begin(),_sortedChildren.end());
	
	unsigned i,j;
	std::string marker;
	Individual* firstTwin,*lastTwin;
	double x,y,startX,endX;
	
	double radius       = DrawingMetrics::getIconRadius();
	double verticalTick = DrawingMetrics::getVerticalTick();
	
	bool twinOnTheLeft=false,twinOnTheRight=false;
	
	for(i=0;i<children.size();i++){
		if(children[i]->getTwinMarker().getMarker() != "."){
			// Get the number of twins that belong to this group
			
			firstTwin = children[i];
			marker = children[i]->getTwinMarker().getMarker();
			for(j=i+1;j<children.size();j++){
				if(children[j]->getTwinMarker().getMarker() == marker) continue;
				break;
			}
			lastTwin = children[j-1];
			
			x = firstTwin->getX()+(lastTwin->getX()-firstTwin->getX())/2;
			y = lastTwin->getY()-radius-DrawingMetrics::getVerticalDrop2();
			if(i==0){
				twinOnTheLeft = true;
				startX = x;
			}
			if(j==children.size()){
				twinOnTheRight = true;
				endX = x;
			}
			
			// Draw the vertical drop2 from the center of the twin Group
			dc.drawVerticalLine( x , y , y + verticalTick );
			
			// Connect each twin in the twin group to the central vertical drop
			for(unsigned k=i;k<j;k++){
				
				dc.drawLine( children[k]->getX() ,
				             children[k]->getY() - radius - verticalTick ,
				             x,
				             y + verticalTick
				           );
				
			}
			
			// If the twins are monozygotic draw the horizontal line connecting them
			if(children[i]->getTwinMarker().getTwinTypeEnum() == Twin::MONOZYGOTIC_TWIN){
				
				dc.drawHorizontalLine( y + DrawingMetrics::getVerticalDrop2() - verticalTick ,
				                       children[ i ]->getX() ,
				                       children[j-1]->getX()
				                     );
				                     
			}
			i=j-1;
			
		}
	}
	if(twinOnTheLeft && twinOnTheRight)
		dc.drawHorizontalLine(y,startX,endX);
	else if(twinOnTheLeft)
		dc.drawHorizontalLine(y,startX,children[children.size()-1]->getX());
	else if(twinOnTheRight)
		dc.drawHorizontalLine(y,children[0]->getX(),endX);
	else dc.drawHorizontalLine(y,children[0]->getX(),children[children.size()-1]->getX());
	 
}

////////////////////////////////////////
//
// PUBLIC METHODS
//
////////////////////////////////////////

//
// sortChildrenInClassicalOrder: 
//
void NuclearFamily::sortChildrenInClassicalOrder(bool consanguinousLoop,bool multipleDT){
	
	std::deque<Individual*> leftLoopIndividuals;
	std::deque<Individual*> rightLoopIndividuals;
	std::deque<Individual*> initial;
	std::deque<Individual*> result;
	
	if(_childrenInClassicalOrder.size() == _sortedChildren.size()){
		_sortedChildren.swap(_childrenInClassicalOrder);
		_childrenInClassicalOrder.clear();
	}
	
	//
	// If the NF has only one child no sorting is required...return.
	//
	if(_sortedChildren.size() == 1) { _childrenInClassicalOrder.push_back(_sortedChildren.front()); return; }
	
	// Group the individuals based on the loop flags
	Individual::groupIndividualsBasedOn(consanguinousLoop,_sortedChildren,initial,leftLoopIndividuals,rightLoopIndividuals);
	
	// Handle Twin Cases:
	if(_twinGroupCount > 0) _rearrangeIndividualsBasedOnTwins(consanguinousLoop,initial,leftLoopIndividuals,rightLoopIndividuals);
	
	// If both the deques are empty nothing more to do return
	
	// For cases with multiple DTs:
	std::deque<unsigned> leftExternal;
	std::deque<unsigned> rightExternal;
	
	// Determine if one of the deques is empty
	if(rightLoopIndividuals.size() == 0){
		
		// Merge the left loop individuals to the right of the inital deque
		while(!leftLoopIndividuals.empty()){
			initial.push_back(leftLoopIndividuals.front());
			leftLoopIndividuals.pop_front();
		}
		result = initial;
		
	}else
	if(leftLoopIndividuals.size() == 0){
		
		// Merge the right loop individuals to the left of the initial deque
		while(!rightLoopIndividuals.empty()){
			initial.push_front(rightLoopIndividuals.back());
			rightLoopIndividuals.pop_back();
		}
		result = initial;
		
	}else
	if(leftLoopIndividuals.back()->getLeftFlag(consanguinousLoop) == rightLoopIndividuals.front()->getRightFlag(consanguinousLoop)){
		
		//
		// NOTE: If was changed to while as there could be repeated individuals on both left and right qs
		// Check that they are infact different individuals
		//
		//if(leftLoopIndividuals.back()->getId() == rightLoopIndividuals.front()->getId()){
		while(leftLoopIndividuals.size() > 0 && rightLoopIndividuals.size() > 0 && leftLoopIndividuals.back()->getId() == rightLoopIndividuals.front()->getId()){
			
			if(rightLoopIndividuals.size() > 1 && leftLoopIndividuals.size() > 1){
				result.push_back(leftLoopIndividuals.back());
				rightLoopIndividuals.pop_front();
				leftLoopIndividuals.pop_back();
			}else
			if(rightLoopIndividuals.size() > 1){
				rightLoopIndividuals.pop_front(); 
			}else if(leftLoopIndividuals.size() > 1){ 
				leftLoopIndividuals.pop_back(); 
			}else{ 
				// Both left and right seem to have only one element in the Deque (NF)
				result.push_back(leftLoopIndividuals.back());
				leftLoopIndividuals.pop_back();
			}
			
		}
		
		if(leftLoopIndividuals.size() > 0 && rightLoopIndividuals.size() > 0 && leftLoopIndividuals.back()->getId() != rightLoopIndividuals.front()->getId()){
			
			result.push_back(leftLoopIndividuals.back());
			result.push_back(rightLoopIndividuals.front());
			
			// delete these two individuals from both the queues
			leftLoopIndividuals.pop_back();
			rightLoopIndividuals.pop_front();
			
			bool leftflag=true;
			// Merge the remaining individuals on the left loop to the LHS of result
			while(!leftLoopIndividuals.empty()){
				leftflag = true;
				// Check if the individual is not already in the result
				for(unsigned j=0;j<result.size();j++){
					if(result[j]->getId() == leftLoopIndividuals.back()->getId()){
						leftLoopIndividuals.pop_back();
						leftflag=false;
						break;
					}
				}
				if(leftflag){ result.push_front(leftLoopIndividuals.back()); leftLoopIndividuals.pop_back(); }
			}
			bool rightflag = true;
			// Merge the remaining individuals on the right loop to the RHS of result
			while(!rightLoopIndividuals.empty()){
				rightflag = true;
				for(unsigned j=0;j<result.size();j++){
					if(result[j]->getId() == rightLoopIndividuals.front()->getId()){
						rightLoopIndividuals.pop_front();
						rightflag=false;
						break;
					}
				}
				if(rightflag){ result.push_back(rightLoopIndividuals.front()); rightLoopIndividuals.pop_front(); }
				
			}
			
			// Merge initial with result
			while(!initial.empty()){
				if(multipleDT){
					// Check for right side external flags
					unsigned resultRightExternal =  result.front()->getRightSideOfExternalConnection();
					unsigned initialRightExternal =  initial.back()->getRightSideOfExternalConnection();
					unsigned resultLeftExternal =  result.back()->getLeftSideOfExternalConnection();
					unsigned initialLeftExternal =  initial.back()->getLeftSideOfExternalConnection();
					if((resultRightExternal > initialRightExternal) || (initialLeftExternal > resultLeftExternal) || (result.front()->isConsanguinous() && !result.back()->hasExternalConnection())){
						result.push_back(initial.back());
						initial.pop_back();
					}else{ 
						result.push_front(initial.back());
						initial.pop_back();
					}
				
				}else{ 
					result.push_front(initial.back());
					initial.pop_back();
				}
			}
		}
		
	}else{
		// push all the left loop individuals to the right of result
		while(!leftLoopIndividuals.empty()){
			initial.push_back(leftLoopIndividuals.front());
			leftLoopIndividuals.pop_front();
		}
		// merge the remaining individuals on the right loop to the left of result
		bool flag;
		while(!rightLoopIndividuals.empty()){
			flag = true;
			for(unsigned j=0;j<initial.size();j++){
				
				if(initial[j]->getId() == rightLoopIndividuals.back()->getId()){
					rightLoopIndividuals.pop_back();
					flag=false;
					break;
				}
			}
			if(flag){ initial.push_front(rightLoopIndividuals.back()); rightLoopIndividuals.pop_back(); }
		}
		
		result = initial;
	}
	
	// Put the result deque in childrenSortedInClassicalOrder vector
	for(unsigned i=0;i<result.size();i++){
		_childrenInClassicalOrder.push_back(result[i]);
	}
	
}

//
// getChildInClassicalOrder:
//
Individual* NuclearFamily::getChildInClassicalOrder(unsigned index){
	
	return _childrenInClassicalOrder[index];
	
}

//
// calculateWidth:
//
void NuclearFamily::calculateWidth(bool classicalOrder){
	
	unsigned i=0,j;
	unsigned totalWidth = 0,rightWidth=0,leftWidth=0;
	
	// Determine which ordering of children to use
	std::vector<Individual*> children;
	if(classicalOrder) children.assign(_childrenInClassicalOrder.begin(),_childrenInClassicalOrder.end());
	else children.assign(_sortedChildren.begin(),_sortedChildren.end());
	
	while(i < children.size()){
		
		if(children[i]->getNumberOfNuclearFamilies()){
			
			// For each nuclear family calculate the width
			for(j=0;j<children[i]->getNumberOfNuclearFamilies();j++){
				
				// Check to see if there is a case of consangunity or external connection
				if(children[i]->getNuclearFamily(j)->isConsanguinous() || children[i]->getNuclearFamily(j)->hasExternalConnection()){
					// Children are always drawn with the mother 
					// So if the child is male continue
					// Set the hasBeenVisited flag
					if(children[i]->getGender().getEnum() == Gender::MALE){
						// Only if the individual has 1 NF
						if(children[i]->getNumberOfNuclearFamilies() == 1)
							totalWidth += 2;
						children[i]->setVisited(true);
						continue;
					}
					else 
					if(children[i]->getNuclearFamily(j)->getFather()->hasBeenVisited() && children[i]->getNumberOfNuclearFamilies() == 1 ){
						
						// NOTE: Commented the if on 2006-11-14; This allows the leftSpouseConnector flag to be set
						// whenever the male spouse is on the left
						// Further checks are performed in the draw code
						//
						// Set left spouse connector flag
						// NOTE: Currently, left connections are handled for a male spouse that has only 1 NF or
						// a male spouse that has all consanguinous and/or external connection relationships
						//
						/*Individual* childFather = children[i]->getNuclearFamily(j)->getFather();
						unsigned cntFatherNF = childFather->getNumberOfNuclearFamilies();
						unsigned currentNFCnt = 0;
						bool fatherCEFlag = true; // flag that indicates if all spouse connections are consanguinous or external
						while(currentNFCnt < cntFatherNF){
							if(childFather->getNuclearFamily(currentNFCnt)->isConsanguinous() || childFather->getNuclearFamily(currentNFCnt)->hasExternalConnection()){
								currentNFCnt++;
							}else{
								fatherCEFlag = false;
								break;
							}
						}
						*/
						// Make sure the  spouse has 1 NF or fatherCEFlag is set
						//if(childFather->getNumberOfSpouses() <= 2 || fatherCEFlag == true){
							children[i]->setLeftSpouseConnector(true);
							//std::cout << "*** SET LEFT SPOUSE *** connector for " << children[i]->getId() << std::endl;
						//}
						
					}
					
				}
				children[i]->getNuclearFamily(j)->calculateWidth(classicalOrder);
				totalWidth+=children[i]->getNuclearFamily(j)->getTotalWidth();
			}
			//
			// If the child has more than one nuclear family store the total,left and right width of all the
			// nuclear families in the child's width
			// 
			if(children[i]->getNumberOfNuclearFamilies() > 1){
				unsigned sumLeftWidth =0,sumRightWidth=0;
				for(j=0;j<children[i]->getNumberOfNuclearFamilies();j++){
					
					// Continue if the individual is a male and has loop flags set
					if(_isMaleWithLoopFlags(children[i],j)){ continue; }
					// The first NF that comes on the right is supposed to be the starting point of drawing this multiple spouse group
					if(sumLeftWidth == 0 && sumRightWidth == 0 && j % 2 == 0){
							sumRightWidth += children[i]->getNuclearFamily(j)->getRightWidth();
							sumLeftWidth +=  children[i]->getNuclearFamily(j)->getLeftWidth();
					}else
					if(j % 2 == 0){
						sumRightWidth += children[i]->getNuclearFamily(j)->getTotalWidth();
					}else{
						sumLeftWidth += children[i]->getNuclearFamily(j)->getTotalWidth();
					}
				}
				// Correct the width when the width on the left or right on the individual is zero:
				if(sumRightWidth == 0 && sumLeftWidth != 0){
					
					// Since this is a multiple spouse case we need to add the extra space between the NFs
					// In multiple spouse case if nf 0 is not consang its width takes care of the width of the individual
					// In cases where the individual has NFs only on the left, we need to add extra width of 2 to 
					// accomodate the individual itself. See eg. amdtest3.txt
					sumLeftWidth += 2;
					totalWidth += 2;
					
				}
				// DEBUG: for individuals with multiple spouses
				//std::cout << "SUM Right width " << sumRightWidth << " SUM Left width " << sumLeftWidth << std::endl;
				//std::cout << "Total Width " << totalWidth << std::endl;
				
				if(sumLeftWidth+sumRightWidth == 0){ 
					
					children[i]->setRightWidth(1);
					children[i]->setLeftWidth(1);
					totalWidth +=2; 
				}else{
					// set the sum of the width on the individual
					children[i]->setRightWidth(sumRightWidth);
					children[i]->setLeftWidth(sumLeftWidth);
				}
			}
		}else{
			//std::cout << "ISOLATE:: " << children[i]->getId() << std::endl;
			totalWidth+=2;
		}
		++i;
	}
	
	// *************************************
	// Calculate LEFT and RIGHT width here:
	// *************************************
	
	unsigned last = children.size()-1;
	// If there is a single child the total width has to include the width of both the parents
	if(children.size() == 1){
		if(totalWidth == 2) totalWidth +=2;
		// No matter what the total width, if the parent has 
		// only one child the leftwidth and rightwidth is always 2
		// This fact can be taken care of in the drawing though
		//
		leftWidth = 2;
		rightWidth = 2;
		// Get the left and right width of the child
		if(children[0]->getNumberOfNuclearFamilies()){
			if(children[0]->getNumberOfNuclearFamilies() == 1){
				if(_isMaleWithLoopFlags(children[0],0));
				else{
					leftWidth = children[0]->getNuclearFamily((unsigned)0)->getLeftWidth();
					rightWidth = children[0]->getNuclearFamily((unsigned)0)->getRightWidth();
				}
			}else{
				if(children[0]->getLeftWidth()+children[0]->getRightWidth() > 2){
					leftWidth = children[0]->getLeftWidth();
					rightWidth = children[0]->getRightWidth();
					// For cases with a single child and rightWidth 0 
					// you need to add some width on the right for the parents
					if(rightWidth == 0){ rightWidth = 2; totalWidth += 2; }
					
				}
			}
		}
	}else
	// Check if the rightmost child has its own NF or if the leftMostChild has more than 1 NF
	if(children[last]->getNumberOfNuclearFamilies()){
		
		//If the rightmost child has children adjust the left and right widths
		// EXCEPTION: If the rightmost child is a male and all his connections are consanguinous/external no adjustments are required
		
		// Check if there is only one NF
		if(children[last]->getNumberOfNuclearFamilies() == 1){
			// If the child is a male with consanguinity or has external connection just do the normal division
			if(_isMaleWithLoopFlags(children[last],0)){
				leftWidth = rightWidth = totalWidth/2;
			}else{
				leftWidth = (totalWidth - children[last]->getNuclearFamily((unsigned)0)->getRightWidth())/2;
				rightWidth = totalWidth - leftWidth;
			}
		}else{
			// The last child has multiple spouses
			// The total width of the group is stored on the individual
			if(children[last]->getRightWidth() == 1){
				// The last child has all consanguinous nuclear families
				// Calculate the widths normally
				leftWidth = rightWidth = totalWidth/2;
			}else
			if(children[last]->getRightWidth() == 0){
				//Case where consang nuclear family is on the RIGHT
			}else{
				leftWidth = (totalWidth - children[last]->getRightWidth())/2;
				rightWidth = totalWidth - leftWidth;
			}
		}
		//std::cout << "RIGHT MANIPULATION : " << leftWidth << " and " << rightWidth << std::endl;
	}
	//******************************
	// LeftWidth manipulation
	//*****************************
	if(children.size() > 1 && children[0]->getNumberOfNuclearFamilies() > 1){
		// The leftmost child has multiple spouses
		// adjust the left and right widths
		
		//std::cout << "LEFT WIDTH MANIPULATION" << std::endl;
		if(children[0]->getLeftWidth() == 2){
			// No left manipulation required since the left width is equal to half the mating pair width
		}else
		if(children[0]->getLeftWidth() == 1 && children[0]->getRightWidth() == 1){
			// The LHS has all consang NFs
		}else
		if(rightWidth == 0 && leftWidth == 0){
			// Handle cases where the RHS has no manipulations
			rightWidth = (totalWidth - children[0]->getLeftWidth())/2;
			if(children[0]->getLeftWidth() % 2 != 0) rightWidth++;
			leftWidth = totalWidth - rightWidth;
		}else{
			if(_isMaleWithLoopFlags(children[0],0)){
				if(children[0]->getLeftWidth() != 0){
					if(children[0]->getRightWidth() == 0){
						leftWidth += (totalWidth - children[0]->getLeftWidth()-1); // 2 is the width of the individual
					}else{
						leftWidth += (totalWidth - children[0]->getLeftWidth());
					}
					rightWidth = totalWidth - leftWidth;
				} 
			}else{
				leftWidth  += ((totalWidth/2) - children[0]->getNuclearFamily((unsigned)0)->getRightWidth())/2;
				rightWidth = totalWidth - leftWidth;
			}
		}
	}else
	if(children.size() > 1 && children[0]->getNumberOfNuclearFamilies() == 1){
		//
		// This check is required for pedigrees whose LHS has a width of > 2 eg. test2.txt
		// Check for Consanguinity  and External Connection
		if(_isMaleWithLoopFlags(children[0],0));
		else if(children[0]->getNuclearFamily((unsigned)0)->getLeftWidth() > 2){
			//
			// Do the manipulation only if the LW of the parent becomes > than
			// the LW of the child NF or if leftWidth == 0
			//
			if(leftWidth == 0) leftWidth = totalWidth/2;
			
			//std::cout << "***ADDITIONAL LEFT width manipulation required for " << children[0]->getId() << std::endl;
			// If rigthwidth > totalwidth/2 added for pedigrees with left skew. 
			if(totalWidth/2 < children[0]->getNuclearFamily((unsigned)0)->getRightWidth() || (rightWidth > totalWidth/2)){
				
				if(leftWidth <= children[0]->getNuclearFamily((unsigned)0)->getLeftWidth()){
					leftWidth +=  children[0]->getNuclearFamily((unsigned)0)->getLeftWidth() - leftWidth+1;
					rightWidth = totalWidth - leftWidth;
				}
			}else{
				
				unsigned offset = ((totalWidth/2) - children[0]->getNuclearFamily((unsigned)0)->getRightWidth())/2;
				// Check if the child's left width > that of parent's
				if(leftWidth+offset < children[0]->getNuclearFamily((unsigned)0)->getLeftWidth()){
					leftWidth = children[0]->getNuclearFamily((unsigned)0)->getLeftWidth();
					rightWidth = totalWidth - leftWidth;
				}else{
					leftWidth += offset;
					rightWidth = totalWidth - leftWidth;
				}
			}
		}
	}

	if(leftWidth == 0 && rightWidth == 0) leftWidth = rightWidth = totalWidth/2;
	if(rightWidth == 0 && leftWidth != 0) rightWidth = totalWidth - leftWidth;
	
	_width.setTotal(totalWidth);
	_width.setLeft(leftWidth);
	_width.setRight(rightWidth);
	
	// DEBUG:
	/*std::cout << std::endl;
	std::cout << "***WIDTH calculation of NF***" << std::endl;
	display();
	std::cout << "Left width for NF is  :" << leftWidth << std::endl;
	std::cout << "Right width for NF is :" << rightWidth << std::endl;
	std::cout << "Total width for NF is :" << totalWidth << std::endl;
	*/
	
}

//
// draw: draws the NuclearFamily
//
void NuclearFamily::draw(Individual* startIndividual,DrawingCanvas& dc,double startX, double startY,bool classicalOrder,bool dashedOriginalFounder){
	
	// NOTE: The children are always drawn with the mother
	//
	// Initialize : get drawing attributes from DrawingMetrics
	//
	double currentX = startX;
	double currentY = startY;
	double horizontalInterval = DrawingMetrics::getHorizontalInterval();
	double iconDiameter       = DrawingMetrics::getIconDiameter();
	double iconInterval       = DrawingMetrics::getIconInterval();
	double verticalDrop1      = DrawingMetrics::getVerticalDrop1();
	double verticalDrop2      = DrawingMetrics::getVerticalDrop2();
	double verticalTick       = DrawingMetrics::getVerticalTick();
	unsigned i=0;
	Individual* spouse;
	
	//
	// Determine which ordering of children to use
	//
	std::vector<Individual*> children;
	if(classicalOrder) children.assign(_childrenInClassicalOrder.begin(),_childrenInClassicalOrder.end());
	else children.assign(_sortedChildren.begin(),_sortedChildren.end());
	
	// If the left connection shift flag is set adjust the currentX
	if(_leftConnectionShiftFlag && !_isMaleWithLoopFlags(startIndividual,0)){
		currentX += horizontalInterval;
	}
	
	//
	// Start by drawing the startIndividual:
	//
	if(startIndividual->getLeftSpouseConnector()){
		dc.drawIndividual(startIndividual,currentX+iconInterval,currentY,dashedOriginalFounder);
	}else{
		// It is possible that the startIndividual is male and has loop flags set; he needs to be drawn with his pedigree
		// rather than his spouse which has multipls NFs
		if((isConsanguinous() || hasExternalConnection()) && startIndividual->getGender().getEnum() == Gender::MALE && _mother->hasBeenDrawn() && !children[0]->hasBeenDrawn());
		else dc.drawIndividual(startIndividual,currentX,currentY,dashedOriginalFounder);
	}
	
	
	if((isConsanguinous() || hasExternalConnection()) && startIndividual->getGender().getEnum() == Gender::MALE){
		if(_mother->hasBeenDrawn() && !children[0]->hasBeenDrawn() && _mother->getNumberOfNuclearFamilies() > 1){
			// Though the father has loop flags set, do not return as the mother has > 1 NF and the children have not been drawn yet
		}else{
			// Do not draw the NF with the father if he has loop flags set
			return;
		}
	}
	
	//
	// Draw spouse connector line: 
	// 
	double x1, x2;
	x1 = currentX + iconDiameter/2; 
	x2 = currentX + iconInterval-iconDiameter/2;
	
	dc.drawLine(x1,currentY,x2,currentY);
	
	//
	// If consanguinous, make a double line:
	//
	if(isConsanguinous()) dc.drawHorizontalLine(currentY-verticalTick,x1,x2);
	
	//
	// Draw the spouse
	//
	if(startIndividual->getId() == _father->getId()){
		spouse = _mother;
	}else spouse = _father;
	// Draw the spouse only if the Nuclear Family is not consanguinous and does not have an external connection:
	if(!isConsanguinous() && !hasExternalConnection()){
		dc.drawIndividual(spouse,currentX+iconInterval,currentY,dashedOriginalFounder);
	}
	
	
	currentX += horizontalInterval;
	
	//
	// Draw the verticalDrop1
	//
	// 2006.09.13.ET: ADDENDUM: ADD CLASS and ID to vertical drop line:
	//
	std::string dropLineId= _mother->getId().get() + std::string(":") + _father->getId().get();
	dc.drawVerticalLine(currentX,currentY,currentY+verticalDrop1,std::string("mating"),dropLineId);
	
	if(_leftConnectionShiftFlag){
		currentX -= horizontalInterval;
	}
	
	//
	// Adjust the currentX based on the left width of the first child's nuclear family
	// NOTE: The left width can be = 1 even in case of multiple NFs due to CONSANG
	//
	if(children[0]->getNumberOfNuclearFamilies() > 1 && children[0]->getLeftWidth() > 1){
		
		//
		// If the LHS child has > 1 spouse currentX has to be modified
		// total left width - left width of the child
		if(children.size() == 1 && _isMaleWithLoopFlags(children[0],0));
		else
		if(_width.getLeft() > children[0]->getLeftWidth()){
			currentX -= (_width.getLeft()-children[0]->getLeftWidth()+1) * horizontalInterval;
		}else if(_width.getLeft() == children[0]->getLeftWidth()) currentX -= horizontalInterval;
		
	}else
	if(children[0]->getNumberOfNuclearFamilies() == 1){
		
		//
		// The cases where the left most
		// child has a single Nuclear family have to be dealt differently
		//
		// Adjust currentX for a NF with a single child who is married
		// so that he gets drawn just under the first parent instead of the center
		if(children.size() == 1 ){
			if((!children[0]->isConsanguinous() && !children[0]->hasExternalConnection()) || children[0]->getGender().getEnum()==Gender::FEMALE){
			
			currentX -= horizontalInterval;
		}
		}else{
			if(_isMaleWithLoopFlags(children[0],0)){
				if(children.size() == 1);
				else{
					currentX -= horizontalInterval * (_width.getLeft() - 1);
				}
			}else{
				if(_width.getLeft() > children[0]->getNuclearFamily((unsigned)0)->getLeftWidth())
					currentX -= (_width.getLeft()-children[0]->getNuclearFamily((unsigned)0)->getLeftWidth()+1) * horizontalInterval;
				else currentX -= iconInterval;
			}
		}
	}else{
		if(children.size() == 1 && (children[0]->isConsanguinous() || children[0]->hasExternalConnection()));
		else currentX -= horizontalInterval * (_width.getLeft() - 1);
	}
	
	currentY += verticalDrop1;
	
	//
	// Draw the children:
	//
	while(i < children.size()){
		
		//std::cout << "Child #  " << children[i]->getId() << std::endl;
		//**************
		// Child has 1 or more nuclear families:
		//**************
		if(children[i]->getNumberOfNuclearFamilies()){
			
			// Check if the child has the left spouse connector flag set:
			if(children[i]->getLeftSpouseConnector()){
				// Even though the leftSpouseConnector is set we need to verify that the male spouse is
				// actually the last to be drawn on his NF. If not we reset the leftconnector flag
				NuclearFamily* lcnf = children[i]->getNuclearFamily(0);
				// Check if the spouse is a single child 
				// If the NF has only one child and the father is to the right reset the leftSpouseConnector flag:
				if(lcnf->getNumberOfChildren() == 1 && lcnf->getFather()->hasBeenDrawn() && lcnf->getFather()->getX() > currentX){
					if(_twinGroupCount && children[i]->getTwinMarker().get() != "."){
						dc.drawVerticalLine(currentX,currentY+verticalDrop2-verticalTick,currentY+verticalDrop2);
					}else if(children[i]->getGender().get() == "."){
						dc.drawVerticalLine(currentX,currentY,currentY+verticalDrop2-(M_SQRT2-1)*iconDiameter);
					}else{
						dc.drawVerticalLine(currentX,currentY,currentY+verticalDrop2);
					}
					//std::cout << " RESET the LEFT SPOUSE CONNECTOR FOR " <<  children[i]->getId() << std::endl;
					children[i]->setLeftSpouseConnector(false);
				}else{
				
					// Draw verticalDrop2 for the child
					// Draw a short vertical drop for twins
					if(_twinGroupCount && children[i]->getTwinMarker().get() != "."){
						dc.drawVerticalLine(currentX+iconInterval,currentY+verticalDrop2-verticalTick,currentY+verticalDrop2);
					}else if(children[i]->getGender().get() == "."){
						dc.drawVerticalLine(currentX+iconInterval,currentY,currentY+verticalDrop2-(M_SQRT2-1)*iconDiameter);
					}else{
						dc.drawVerticalLine(currentX+iconInterval,currentY,currentY+verticalDrop2);
					}
				}
			}
			else if(children[i]->getNuclearFamily((unsigned)0)->getLeftConnectionShiftFlag() && !_isMaleWithLoopFlags(children[i],0)){
				if(_twinGroupCount && children[i]->getTwinMarker().get() != "."){
					dc.drawVerticalLine(currentX+horizontalInterval,currentY+verticalDrop2-verticalTick,currentY+verticalDrop2);
				}else if(children[i]->getGender().get() == "."){
					dc.drawVerticalLine(currentX+horizontalInterval,currentY,currentY+verticalDrop2-(M_SQRT2-1)*iconDiameter);
				}else{ 
					dc.drawVerticalLine(currentX+horizontalInterval,currentY,currentY+verticalDrop2);
				}
			}else if(_twinGroupCount && children[i]->getTwinMarker().get() != ".")
				dc.drawVerticalLine(currentX,currentY+verticalDrop2-verticalTick,currentY+verticalDrop2);
			else{
				if(children[i]->getGender().get() == "."){
					dc.drawVerticalLine(currentX,currentY,currentY+verticalDrop2-(M_SQRT2-1)*iconDiameter);
				}else{ 
					dc.drawVerticalLine(currentX,currentY,currentY+verticalDrop2);
				}
			}
			
			double xl = currentX - (children[i]->getNuclearFamily((unsigned)0)->getLeftWidth()) * horizontalInterval;
			double xr = currentX;
			for(unsigned j=0;j<children[i]->getNumberOfNuclearFamilies();j++){
				if(j % 2 == 0){
					// If the child is a male who has been drawn and the current NF is consang continue
					if(children[i]->hasBeenDrawn() && _isMaleWithLoopFlags(children[i],j) == true && children[i]->getNumberOfNuclearFamilies() > 1){ continue; }
					else{
						if(j != 0) xr += children[i]->getNuclearFamily(j)->getLeftWidth() * horizontalInterval;
						children[i]->getNuclearFamily(j)->draw(children[i],dc,xr,currentY+verticalDrop2+iconDiameter/2,classicalOrder);
						 xr += children[i]->getNuclearFamily(j)->getRightWidth() * horizontalInterval;
					}
				}else{
					
					xl -= children[i]->getNuclearFamily(j)->getRightWidth() * horizontalInterval;
					if(children[i]->getId() != children[i]->getNuclearFamily(j)->getMother()->getId())
						spouse = children[i]->getNuclearFamily(j)->getMother();
					else 
						spouse = children[i]->getNuclearFamily(j)->getFather();
					if(spouse->getGender().getEnum() == Gender::FEMALE)
					// If the spouse is a female and the NF has a loop flag set continue:
					if((spouse->hasBeenDrawn()  || children[i]->getNuclearFamily(j)->isConsanguinous() || children[i]->getNuclearFamily(j)->hasExternalConnection()) && spouse->getGender().getEnum() == Gender::FEMALE){
						//std::cout << "This spouse has already been drawn!!! " << spouse->getId() << std::endl;
						continue;
					}
					
					children[i]->getNuclearFamily(j)->draw(spouse,dc,xl,currentY+verticalDrop2+iconDiameter/2,classicalOrder);
					xl -= (children[i]->getNuclearFamily(j)->getLeftWidth())  * horizontalInterval;
				}
			}
			if(children[i]->getNumberOfNuclearFamilies() >= 2) drawSpouseConnectors(children[i],horizontalInterval,iconInterval,iconDiameter,dc);
			
			//**********************************
			//
			// Adjust the currentX and currentY to draw the next sibling:
			//
			//**********************************
			
			//
			// The next child has NFs:
			//
			if(i+1 < children.size() && children[i+1]->getNumberOfNuclearFamilies()){
				//
				// The current child has more than 1 NF
				//
				if(children[i]->getNumberOfNuclearFamilies() > 1){
					// The current child had multiple spouses and the next child also has multiple spouses
					// This case could occur: eg test2.txt
					if(children[i+1]->getNumberOfNuclearFamilies() > 1){
						currentX += (children[i]->getRightWidth()+children[i+1]->getLeftWidth()) * horizontalInterval;
						// For cases where all the NFs are not drawn with the male
						if(children[i+1]->getLeftWidth() == 1)
							currentX += horizontalInterval;
					}else{
						// The next child has 1 NF
						if(_isMaleWithLoopFlags(children[i+1],0)){
							currentX += children[i]->getRightWidth() * horizontalInterval + iconInterval;
						}else{ 
							// Check if the rw of the current child is 0 implying no nf on the right
							if(children[i]->getRightWidth() == 0){
								if(children[i+1]->getNuclearFamily((unsigned)0)->getLeftWidth() == 2)
									currentX += iconInterval;
								else
									currentX += (children[i+1]->getNuclearFamily((unsigned)0)->getLeftWidth()) * horizontalInterval+horizontalInterval;
							}else{
								currentX += (children[i]->getRightWidth()+children[i+1]->getNuclearFamily((unsigned)0)->getLeftWidth()) * horizontalInterval;
							}
						}
					}
				}else{
					//
					// The current child has 1 NF: 
					//
					if(children[i+1]->getNumberOfNuclearFamilies() > 1){
						// Next child has > 1 NF
						if(_isMaleWithLoopFlags(children[i],0)){
							currentX += children[i+1]->getLeftWidth() * horizontalInterval;
						}else{
							// For cases where all the NFs are not drawn with the male
							if(children[i+1]->getLeftWidth() == 1)
								currentX += children[i]->getNuclearFamily((unsigned)0)->getRightWidth() * horizontalInterval + iconInterval;
							else
								currentX += (children[i]->getNuclearFamily((unsigned)0)->getRightWidth()+children[i+1]->getLeftWidth()) * horizontalInterval;
						}
					}else{
						// Both the Current && Next child have 1 NF
						
						if(_isMaleWithLoopFlags(children[i],0)){
							if(_isMaleWithLoopFlags(children[i+1],0)){
								currentX += iconInterval;
							}else   currentX += children[i+1]->getNuclearFamily((unsigned)0)->getLeftWidth() * horizontalInterval;
						}else{
							if(_isMaleWithLoopFlags(children[i+1],0)){
								currentX += children[i]->getNuclearFamily((unsigned)0)->getRightWidth() * horizontalInterval + iconInterval;
								//if((children[i]->getNuclearFamily((unsigned)0)->getRightWidth() % 2) == 0) currentX += horizontalInterval;
							}else{
								currentX += (children[i]->getNuclearFamily((unsigned)0)->getRightWidth()+children[i+1]->getNuclearFamily((unsigned)0)->getLeftWidth()) * horizontalInterval;
								
							}
						}
					}
				}
			}else if(i+1 < children.size()){
				//
				// The next child does not have a NF
				//
				if(children[i]->getNumberOfNuclearFamilies() > 1){
					// Current child has > 1 NF
					if(_isMaleWithLoopFlags(children[i],0)){
						currentX += iconInterval;
					}else
						currentX += horizontalInterval * children[i]->getRightWidth() + iconInterval;
					
				}else{
					// Current child has 1 NF
					if(_isMaleWithLoopFlags(children[i],0)){
						currentX += iconInterval;
					}else   currentX += horizontalInterval * children[i]->getNuclearFamily((unsigned)0)->getRightWidth() + iconInterval;
				}
			}
		}else{
			//******************
			// Isolate case : 
			//******************
			
			if(children.size() == 1){
				// Single child with no NF adjust the currentX
				// The child should be drawn exactly in the middle
				currentX += horizontalInterval;
			}
			
			//
			// Draw verticalDrop2 for the child
			// Draw a short verticalDrop2 for twins
			//
			if(children[i]->isSpecial()) ;
			else if(_twinGroupCount && children[i]->getTwinMarker().get() != "."){
				dc.drawVerticalLine(currentX,currentY+verticalDrop2-DrawingMetrics::getVerticalTick(),currentY+verticalDrop2);
			}else if(children[i]->getGender().get() == "."){
				dc.drawVerticalLine(currentX,currentY,currentY+verticalDrop2-(M_SQRT2-1)*iconDiameter);
			}else{
				dc.drawVerticalLine(currentX,currentY,currentY+verticalDrop2);
			}
			//
			// Draw the child
			//
			dc.drawIndividual(children[i],currentX,currentY+verticalDrop2+iconDiameter/2);
			
			//
			// Compute currentX and currentY:
			//
			if(i+1 < children.size() && children[i+1]->getNumberOfNuclearFamilies()){
				// Next child has NFs
				if(children[i+1]->getNumberOfNuclearFamilies() > 1){
					// Next child has > 1 NF
					if(children[i+1]->getLeftWidth() == 1){
						// The child has CONSANG
						currentX += iconInterval;
					}else
						currentX += (children[i+1]->getLeftWidth() * horizontalInterval);
				}else{
					// Next child has 1 NF
					if(_isMaleWithLoopFlags(children[i+1],0))
						currentX += iconInterval;
					else{
						currentX += (children[i+1]->getNuclearFamily((unsigned)0)->getLeftWidth() * horizontalInterval);
					}
				}
			}else{
				// Next child is also isolate ie does not have a NF
				currentX += iconInterval;
			}
		}
		i++;
	}
	
	//
	// Draw the line that connects the children
	// The individual stores its co-ordinate position this task can be easily accomplished.
	// 
	
	//Draw the line for the twinGroups
	if(_twinGroupCount) _drawTwinConnectors(dc,classicalOrder);
	else
	if(children.size() == 1 && children[0]->getNumberOfNuclearFamilies()){
		if(children[0]->getLeftSpouseConnector()){
			dc.drawHorizontalLine(children[0]->getY()-verticalDrop2-iconDiameter/2,children[0]->getX(),children[0]->getX()-horizontalInterval);
		}else{
		// *** COMMENTED NOW
		//	if(_isMaleWithLoopFlags(children[0],0) || children[0]->getNuclearFamily((unsigned)0)->getLeftConnectionShiftFlag() || (children[0]->getNumberOfNuclearFamilies() > 1 && children[0]->getGender().getEnum() == Gender::MALE));
			if(_isMaleWithLoopFlags(children[0],0) || children[0]->getNuclearFamily((unsigned)0)->getLeftConnectionShiftFlag());
			else dc.drawHorizontalLine(children[0]->getY()-verticalDrop2-iconDiameter/2,children[0]->getX(),children[0]->getX()+horizontalInterval);
		}
	}else
	if(children.size() > 1)
		dc.drawHorizontalLine(children[0]->getY()-verticalDrop2-iconDiameter/2,children[0]->getX(),children[children.size()-1]->getX());
	
}

//
// drawSpouseConnectors: draws the spouse connections for an individual with > 1 spouse
//
void NuclearFamily::drawSpouseConnectors(Individual* individual,const double horizontalInterval,const double iconInterval,const double iconDiameter,DrawingCanvas& dc){
	
	double xr,yr;
	double radius = iconDiameter/2;
	xr = individual->getX();
	yr  = individual->getY();
	
	double verticalTick=DrawingMetrics::getVerticalTick();
	
	// NOTE: The distance between the 2 lines for Consanguinous connectors is 1 scalingFactor unit.
	
	if(individual->getNumberOfNuclearFamilies() >= 2){
		//
		// Draw the first LHS connector
		//
		if(_isMaleWithLoopFlags(individual,1));
		else{
			dc.drawHorizontalLine(yr,xr-radius,xr-radius+iconInterval-(individual->getNuclearFamily((unsigned)0)->getLeftWidth()+individual->getNuclearFamily((unsigned)1)->getRightWidth())*horizontalInterval);
			if(individual->getNuclearFamily((unsigned)1)->isConsanguinous())
				dc.drawHorizontalLine(yr-verticalTick,xr-radius,xr-radius+iconInterval-(individual->getNuclearFamily((unsigned)0)->getLeftWidth()+individual->getNuclearFamily((unsigned)1)->getRightWidth())*horizontalInterval);
		}
	}
	//
	// Draw the LHS extension line when #NFs > 3
	//
	if(individual->getNumberOfNuclearFamilies() > 3){
		// Check if the individual is a male and all the NFs on the LHS are consanguinous
		// In such cases the extension line is not required
		unsigned i=3;
		if(individual->getGender().getEnum() == Gender::MALE){
			while(i < individual->getNumberOfNuclearFamilies()){
				if(individual->getNuclearFamily(i)->isConsanguinous() || individual->getNuclearFamily(i)->hasExternalConnection()) i+= 2;
				else break;
			}
		}
		// LHS has more than 1 NF draw the extension line:
		if(i >= individual->getNumberOfNuclearFamilies());
		else{
			if(individual->getNuclearFamily(1)->isConsanguinous()){
				dc.drawHorizontalLine(yr-2*verticalTick,xr-radius,xr-horizontalInterval);
				dc.drawVerticalLine(xr-horizontalInterval,yr-2*verticalTick,yr-radius-verticalTick);
			}else{
				dc.drawHorizontalLine(yr-verticalTick,xr-radius,xr-horizontalInterval);
				dc.drawVerticalLine(xr-horizontalInterval,yr-verticalTick,yr-radius-verticalTick);
			}
		}
	}
	
	if(individual->getNumberOfNuclearFamilies() > 2){
		
		// RHS has more than 1 NF draw the extension line
		// Check if the individual is a male and all the NFs on the RHS are consanguinous
		// In such cases the extension line is not required
		unsigned i=2;
		if(individual->getGender().getEnum() == Gender::MALE){
			while(i < individual->getNumberOfNuclearFamilies()){
				if(individual->getNuclearFamily(i)->isConsanguinous() || individual->getNuclearFamily(i)->hasExternalConnection()) i+= 2;
				else break;
			}
		}
		// RHS has more than 1 NF draw the extension line:
		if(i >= individual->getNumberOfNuclearFamilies());
		else{
			if(individual->getNuclearFamily((unsigned)0)->isConsanguinous()){
				dc.drawHorizontalLine(yr-2*verticalTick,xr+radius,xr+horizontalInterval);
				dc.drawVerticalLine(xr+horizontalInterval,yr-2*verticalTick,yr-verticalTick-radius);
			}else{
				dc.drawHorizontalLine(yr-verticalTick,xr+radius,xr+horizontalInterval);
				dc.drawVerticalLine(xr+horizontalInterval,yr-verticalTick,yr-verticalTick-radius);
			}
		}
		double lw=0,rw=0,initialLW,initialRW;
		lw = individual->getNuclearFamily((unsigned)1)->getTotalWidth()+individual->getNuclearFamily((unsigned)0)->getLeftWidth()-2;
		initialLW = lw;
		rw = individual->getNuclearFamily((unsigned)0)->getRightWidth();
		initialRW = rw;
		unsigned nnf = individual->getNumberOfNuclearFamilies();
		
		double offset=0.0;
		// This loops does the summation of the nf widths  to determine the position of the next spouse
		// and draws the extension line based on the calculations
		for(unsigned i=2;i<nnf;i++){
			if(_isMaleWithLoopFlags(individual,i)) continue;
			if(individual->getNuclearFamily(i)->getLeftConnectionShiftFlag()) offset = horizontalInterval;
			else offset = 0.0;
			if(i % 2 == 0){
				rw += individual->getNuclearFamily(i)->getLeftWidth();
				dc.drawVerticalLine(individual->getX()+rw*horizontalInterval+radius+offset,yr-verticalTick-radius,yr);
				if(i+2 < nnf && !(individual->getNuclearFamily(i+2)->isConsanguinous() || individual->getNuclearFamily(i+2)->hasExternalConnection()))
					rw += individual->getNuclearFamily(i)->getRightWidth();
			}else{
				lw += individual->getNuclearFamily(i)->getRightWidth();
				dc.drawVerticalLine(individual->getX()-lw*horizontalInterval-radius-offset,yr-verticalTick-radius,yr);
				//dc.drawVerticalLine(individual->getX()-lw*horizontalInterval+iconInterval-radius),yr-verticalTick-radius),yr));
				if(i+2 < nnf && !(individual->getNuclearFamily(i+2)->isConsanguinous() || individual->getNuclearFamily(i+2)->hasExternalConnection()))
					lw += individual->getNuclearFamily(i)->getLeftWidth();
			}
			
		}
		if(rw && rw != initialRW) dc.drawHorizontalLine(yr-verticalTick-radius,xr+horizontalInterval,xr+rw*horizontalInterval+radius+offset);
		if(nnf > 3 && lw && lw != initialLW)  dc.drawHorizontalLine(yr-verticalTick-radius,xr-horizontalInterval,xr-lw*horizontalInterval-radius-offset);
		
	}
	
}

//
// sortChildrenBasedOnDataField:
//
void NuclearFamily::sortChildrenBasedOnDataField(const std::string& name,bool dobSortOrder){
	
	
	std::vector<Individual*> temp = Individual::sortIndividualsBasedOnDataField(name,_sortedChildren,dobSortOrder);
	_sortedChildren.swap(temp);
	
}

//
// DEBUG:
//
void NuclearFamily::display(){
	
	std::cout << "NUCLEAR FAMILY" << std::endl;
	std::cout << "Mother :"  << _mother->getId() << " Father : " << _father->getId() << std::endl;
	std::cout << "Children : " << std::endl;
	for(unsigned i=0;i<_sortedChildren.size();i++){
		std::cout << _sortedChildren[i]->getId() << std::endl;
	}
	std::cout << "END OF NUCLEAR FAMILY " << std::endl;
	
}

// NOTE: CURRENTLY NOT USED
// getChildrenIds:
//
/*std::vector<std::string> NuclearFamily::getChildrenIds(){
	
	// Check to see if the childrenInClassicalOrder vector is empty. 
	// If it is empty we are dealing with a simple case without consanguinity or external connection.
	// NOTE: currently the children are sorted by Id. But it can be extended to sort by any other field and
	// return the children vector.
	std::vector<std::string> temp;
	if(!_childrenInClassicalOrder.size()){
		for(unsigned i=0;i<_sortedChildren.size();i++){
			temp.push_back(_sortedChildren[i]->getId().get());
		}
	}else{
		for(unsigned i=0;i<_childrenInClassicalOrder.size();i++){
			temp.push_back(_childrenInClassicalOrder[i]->getId().get());
		}
	}
	return temp;
}
*/
