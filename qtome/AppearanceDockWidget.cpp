#include "Stable.h"

#include "AppearanceDockWidget.h"

QAppearanceWidget::QAppearanceWidget(QWidget* pParent, QTransferFunction* tran, CScene* scene) :
	QWidget(pParent),
	m_MainLayout(),
	//m_PresetsWidget(NULL, "Appearance", "Appearance"),
	m_AppearanceSettingsWidget(nullptr, tran, scene)
	//m_TransferFunctionWidget(),
	//m_NodeSelectionWidget(),
	//m_NodePropertiesWidget()
{
	// Create main layout
	m_MainLayout.setAlignment(Qt::AlignTop);
	setLayout(&m_MainLayout);

	QScrollArea* scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(&m_AppearanceSettingsWidget);


	//m_MainLayout.addWidget(&m_PresetsWidget, 0, 0);
	m_MainLayout.addWidget(scrollArea, 1, 0);
	//m_MainLayout.addWidget(&m_TransferFunctionWidget, 2, 0);
	//m_MainLayout.addWidget(&m_NodeSelectionWidget, 3, 0);
	//m_MainLayout.addWidget(&m_NodePropertiesWidget, 4, 0);
	
	//QObject::connect(&m_PresetsWidget, SIGNAL(LoadPreset(const QString&)), this, SLOT(OnLoadPreset(const QString&)));
	//QObject::connect(&gStatus, SIGNAL(LoadPreset(const QString&)), &m_PresetsWidget, SLOT(OnLoadPreset(const QString&)));
	//QObject::connect(&m_PresetsWidget, SIGNAL(SavePreset(const QString&)), this, SLOT(OnSavePreset(const QString&)));
}

void QAppearanceWidget::OnLoadPreset(const QString& Name)
{
	//m_PresetsWidget.LoadPreset(gTransferFunction, Name);
}

void QAppearanceWidget::OnSavePreset(const QString& Name)
{
	//QTransferFunction Preset = gTransferFunction;
	//Preset.SetName(Name);

	// Save the preset
	//m_PresetsWidget.SavePreset(Preset);
}

QAppearanceDockWidget::QAppearanceDockWidget(QWidget *parent, QTransferFunction* tran, CScene* scene) :
	QDockWidget(parent),
	m_VolumeAppearanceWidget(nullptr, tran, scene)
{
	setWindowTitle("Appearance");
	setToolTip("<img src=':/Images/palette.png'><div>Volume Appearance</div>");
	setWindowIcon(GetIcon("palette"));

	m_VolumeAppearanceWidget.setParent(this);

	setWidget(&m_VolumeAppearanceWidget);
}