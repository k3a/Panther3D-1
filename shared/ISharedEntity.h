//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (17.7.2007)
// Purpose:	Shared (cross module) entity interface with basic virtual methods
//-----------------------------------------------------------------------------

class eBase;

class ISharedEntity
{
public:
	virtual ~ISharedEntity(){};
	ISharedEntity(){
		m_bMatrixChanged=true;
		m_pParent = NULL;
	};
	void SetWorldMatrix(P3DXMatrix &mat){ if(m_pParent) return; m_matrix=mat; m_bMatrixChanged=true;};
	void GetWorldMatrix(P3DXMatrix &mat)const{mat=m_matrix;};
	bool WasMatrixChanged()const{return (m_bMatrixChanged || m_bMatrixChangedPrev);}; // Timto jde detekovat zmenu matice v Think metodach entit
protected:
	P3DXMatrix m_matrix;
	P3DXMatrix m_relMatrix;
	bool m_bMatrixChanged;
	bool m_bMatrixChangedPrev; // takovy maly hack. Zde bude info z predchoziho snimku - nutne kvuli parentum
	//
	eBase* m_pParent;
};