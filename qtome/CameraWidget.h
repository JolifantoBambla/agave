#pragma once

#include "Camera.h"
#include "FilmWidget.h"
#include "ApertureWidget.h"
#include "ProjectionWidget.h"
#include "FocusWidget.h"
//#include "PresetsWidget.h"

class CScene;

class QCameraWidget : public QWidget
{
    Q_OBJECT

public:
    QCameraWidget(QWidget* pParent = NULL, QCamera* cam = nullptr, CScene* scene = nullptr);

	virtual QSize sizeHint() const;

private:
	QGridLayout					m_MainLayout;
	QFilmWidget					m_FilmWidget;
	QApertureWidget				m_ApertureWidget;
	QProjectionWidget			m_ProjectionWidget;
	QFocusWidget				m_FocusWidget;
	//QPresetsWidget<QCamera>		m_PresetsWidget;
};