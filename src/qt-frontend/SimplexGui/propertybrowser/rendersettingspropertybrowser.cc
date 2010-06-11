//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2010  Sebastian Mach (*1983)
// * mail: phresnel/at/gmail/dot/com
// * http://phresnel.org
// * http://picogen.org
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <QStringList>
#include <QMessageBox>
#include <QMdiArea>

#include <QtTreePropertyBrowser>
#include <QtProperty>

#include <QtGroupPropertyManager>
#include <QtEnumPropertyManager>
#include <QtStringPropertyManager>

#include <QtVariantPropertyManager>
#include <QtVariantEditorFactory>

#include <QtLineEditFactory>

#include "redshift/include/jobfile.hh"
#include "propertybrowser-helpers.hh"

#include "coloreditmanager.h"
#include "coloreditfactory.h"
#include "colorpicker.hh"

#include "rendersettingspropertybrowser.hh"



RenderSettingsPropertyBrowser::RenderSettingsPropertyBrowser(
        QWidget *ownerWidget_,
        QtTreePropertyBrowser *root
)
: QObject(ownerWidget_)
, ownerWidget(ownerWidget_)
, root(root)
, volumeTypeEnumManager(0)
, surfaceIntegratorTypeEnumManager(0)
, enumManager(0)
, groupManager(new QtGroupPropertyManager(this))
, variantManager(0)
, variantFactory(0)
, renderSettingsProperty(0)
, lineEditFactory(0)
, comboBoxFactory(0)
, rsTitleManager(0)
{
        initializeScene();
}



RenderSettingsPropertyBrowser::~RenderSettingsPropertyBrowser() {
        root->unsetFactoryForManager(variantManager);
        root->unsetFactoryForManager(rsTitleManager);
        root->unsetFactoryForManager(surfaceIntegratorTypeEnumManager);
        root->unsetFactoryForManager(enumManager);

        delete volumeTypeEnumManager;
        delete surfaceIntegratorTypeEnumManager;
        delete enumManager;
        delete groupManager;
        delete variantManager;
        delete variantFactory;

        delete lineEditFactory;
        delete comboBoxFactory;

        delete rsTitleManager;
}



void RenderSettingsPropertyBrowser::initializeScene() {
        variantManager = new QtVariantPropertyManager(this);
        variantFactory = new QtVariantEditorFactory(this);

        connect (variantManager, SIGNAL(valueChanged(QtProperty*,QVariant)),
                 this, SLOT(variantManager_valueChanged(QtProperty*,QVariant)));


        QtProperty *topItem = groupManager->addProperty("render-settings");
        renderSettingsProperty = topItem;
        root->addProperty(topItem);

        root->setBackgroundColor(
                     root->topLevelItem(renderSettingsProperty),
                     QColor(130,90,90));

        rsTitleManager = new QtStringPropertyManager (this);
        connect(rsTitleManager, SIGNAL(valueChanged (QtProperty *, const QString &)),
                this, SLOT(rsTitleManager_valueChanged(QtProperty*,const QString &)));

        surfaceIntegratorTypeEnumManager = new QtEnumPropertyManager(this);
        connect(surfaceIntegratorTypeEnumManager, SIGNAL(valueChanged (QtProperty *, int)),
                this, SLOT(surfaceIntegratorTypeEnumManager_valueChanged(QtProperty*,int)));

        enumManager = new QtEnumPropertyManager(this);
        connect (enumManager, SIGNAL(valueChanged(QtProperty*,int)),
                 this, SLOT(enumManager_valueChanged(QtProperty*,int)));


        comboBoxFactory = new QtEnumEditorFactory(this);
        lineEditFactory = new QtLineEditFactory(this);

        root->setFactoryForManager(variantManager, variantFactory);
        root->setFactoryForManager(rsTitleManager, lineEditFactory);
        root->setFactoryForManager(surfaceIntegratorTypeEnumManager, comboBoxFactory);
        root->setFactoryForManager(enumManager, comboBoxFactory);
}



void RenderSettingsPropertyBrowser::addRenderSettings (
        redshift::scenefile::RenderSettings const &rs
) {

        const bool block = blockSignals(true);
        QtProperty *topItem = groupManager->addProperty(QString::fromStdString(rs.title));
        renderSettingsProperty->addSubProperty(topItem);

        QtProperty *title = rsTitleManager->addProperty("title");
        rsTitleManager->setRegExp(title, QRegExp("([a-z0-9]|-|_)+", Qt::CaseInsensitive, QRegExp::RegExp));
        // here it breaks
        rsTitleManager->setValue(title, QString::fromStdString(rs.title));
        topItem->addSubProperty(title);

        QtVariantProperty *it = variantManager->addProperty(QVariant::Int, "width");
        it->setAttribute(QLatin1String("minimum"), 1);
        it->setAttribute(QLatin1String("maximum"), 0xFFFFFF);
        it->setAttribute(QLatin1String("singleStep"), 1);
        it->setValue(rs.width);
        topItem->addSubProperty(it);

        it = variantManager->addProperty(QVariant::Int, "height");
        it->setAttribute(QLatin1String("minimum"), 1);
        it->setAttribute(QLatin1String("maximum"), 0xFFFFFF);
        it->setAttribute(QLatin1String("singleStep"), 1);
        it->setValue(rs.height);
        topItem->addSubProperty(it);

        it = variantManager->addProperty(QVariant::Int, "samples-per-pixel");
        it->setAttribute(QLatin1String("minimum"), 1);
        it->setAttribute(QLatin1String("maximum"), 0xFFFFFF);
        it->setAttribute(QLatin1String("singleStep"), 1);
        it->setValue(rs.samplesPerPixel);
        topItem->addSubProperty(it);

        it = variantManager->addProperty(QVariant::Int, "min-y");
        it->setAttribute(QLatin1String("minimum"), 0);
        it->setAttribute(QLatin1String("singleStep"), 1);
        it->setValue(rs.min_y);
        topItem->addSubProperty(it);

        it = variantManager->addProperty(QVariant::Int, "max-y");
        it->setAttribute(QLatin1String("minimum"), 0);
        it->setAttribute(QLatin1String("singleStep"), 1);
        it->setValue(rs.max_y);
        topItem->addSubProperty(it);

        it = variantManager->addProperty(QVariant::Int, "seed");
        it->setAttribute(QLatin1String("minimum"), 0);
        it->setAttribute(QLatin1String("singleStep"), 1);
        it->setValue(rs.userSeed);
        it->setToolTip("Use this to configure another random seed for rendering.");
        topItem->addSubProperty(it);

        {
                QtProperty *surfaceIntegrator = groupManager
                                                ->addProperty("surface-integrator");
                topItem->addSubProperty(surfaceIntegrator);

                QtProperty *integratorType = surfaceIntegratorTypeEnumManager->addProperty("type");
                QStringList enumNames;
                enumNames << "none" << "whitted" << "whitted_ambient" << "path";
                surfaceIntegratorTypeEnumManager->setEnumNames(integratorType, enumNames);

                surfaceIntegrator->addSubProperty(integratorType);


                switch (rs.surfaceIntegrator.type) {
                case redshift::scenefile::SurfaceIntegrator::none:
                        surfaceIntegratorTypeEnumManager->setValue(integratorType, 0);
                        surfaceIntegratorTypeEnumManager_valueChanged(integratorType, 0);
                        break;
                case redshift::scenefile::SurfaceIntegrator::whitted:
                        surfaceIntegratorTypeEnumManager->setValue(integratorType, 1);
                        surfaceIntegratorTypeEnumManager_valueChanged(integratorType, 1);
                        break;
                case redshift::scenefile::SurfaceIntegrator::whitted_ambient:
                        surfaceIntegratorTypeEnumManager->setValue(integratorType, 2);
                        surfaceIntegratorTypeEnumManager_valueChanged(integratorType, 2);
                        break;
                case redshift::scenefile::SurfaceIntegrator::path:
                        surfaceIntegratorTypeEnumManager->setValue(integratorType, 3);
                        surfaceIntegratorTypeEnumManager_valueChanged(integratorType, 3);
                        break;
                default:
                        QMessageBox::warning(ownerWidget,
                          "Unsupported Surface Integrator",
                          "It has been tried to load a surface integrator "
                          "\""
                          + QString::fromStdString(redshift::scenefile::SurfaceIntegrator::Typenames[rs.surfaceIntegrator.type])
                          + "\", but this type is currently not supported here."
                        );
                        surfaceIntegratorTypeEnumManager->setValue(integratorType, 0);
                        surfaceIntegratorTypeEnumManager_valueChanged(integratorType, 0);
                }
        }
        {
                QtProperty *volumeIntegrator = groupManager->addProperty("volume-integrator");
                topItem->addSubProperty(volumeIntegrator);

                QtProperty *integratorType = enumManager->addProperty("type");
                QStringList enumNames;
                enumNames << "none" << "emission" << "single";
                enumManager->setEnumNames(integratorType, enumNames);
                volumeIntegrator->addSubProperty(integratorType);
                it = variantManager->addProperty(QVariant::Double, "step-size");
                it->setAttribute(QLatin1String("minimum"), 1.);
                //it->setAttribute(QLatin1String("maximum"), 32768.);
                it->setAttribute(QLatin1String("singleStep"), 1.);
                it->setAttribute(QLatin1String("decimals"), 1);
                it->setValue(rs.volumeIntegrator.stepSize);
                volumeIntegrator->addSubProperty(it);

                it = variantManager->addProperty(QVariant::Double, "cutoff-distance");
                it->setAttribute(QLatin1String("minimum"), 1.);
                //it->setAttribute(QLatin1String("maximum"), 32768.);
                it->setAttribute(QLatin1String("singleStep"), 1.);
                it->setAttribute(QLatin1String("decimals"), 1);
                it->setValue(rs.volumeIntegrator.cutoffDistance);
                volumeIntegrator->addSubProperty(it);


                switch (rs.volumeIntegrator.type) {
                case redshift::scenefile::VolumeIntegrator::none:
                        enumManager->setValue(integratorType, 0);
                        break;
                case redshift::scenefile::VolumeIntegrator::emission:
                        enumManager->setValue(integratorType, 1);
                        break;
                case redshift::scenefile::VolumeIntegrator::single:
                        enumManager->setValue(integratorType, 2);
                        break;
                default:
                        QMessageBox::warning(ownerWidget,
                          "Unsupported Volume Integrator",
                          "It has been tried to load a volume integrator "
                          "\""
                          + QString::fromStdString(redshift::scenefile::VolumeIntegrator::Typenames[rs.volumeIntegrator.type])
                          + "\", but this type is currently not supported here."
                        );
                        enumManager->setValue(integratorType, 0);
                }
        }

        collapse (root, topItem);
        blockSignals(block);

        if (!signalsBlocked()) emit sceneChanged();
}



void RenderSettingsPropertyBrowser::remove(QtProperty *property) {
        renderSettingsProperty->removeSubProperty(property);
        if (!signalsBlocked()) emit sceneChanged();
}



void RenderSettingsPropertyBrowser::rsTitleManager_valueChanged (
        QtProperty * property,
        const QString & value
) {
        if (QtProperty *par = findParent (root->properties(), property)) {
                par->setPropertyName(value);
        }
        if (!signalsBlocked())
                emit sceneChanged();
}



void RenderSettingsPropertyBrowser::surfaceIntegratorTypeEnumManager_valueChanged(
        QtProperty* prop, int index
){
        //emit sceneChanged(); //setChanged();
        QtProperty *t = findParent(root->properties(), prop);
        if (!t)
                return;

        const QStringList enumNames =
                        surfaceIntegratorTypeEnumManager->enumNames(prop);
        const QString type = enumNames[index];

        // Remove all properties not titled "type" to make place for the right
        // properties.
        // TODO: maybe just make them invisible if that is possible.
        foreach (QtProperty *nt, t->subProperties()){
                if (nt->propertyName() != "type") {
                        t->removeSubProperty(nt);
                }
        }

        if (type == "none") {
                // has no options.
        } else if (type == "whitted_ambient") {
                QtVariantProperty* numSamples = variantManager->addProperty(QVariant::Int,"ambient-samples");
                numSamples->setToolTip("Low values give a noisy look, but yield faster renderings.");
                numSamples->setAttribute(QLatin1String("minimum"), 1);
                numSamples->setAttribute(QLatin1String("maximum"), 0xFFFF);
                numSamples->setValue(10);
                t->addSubProperty(numSamples);
        } else if (type == "whitted") {
                // has no options.
        } else if (type == "path") {
                // has no options.
        } else {
                QMessageBox::critical(ownerWidget, "Unsupported object",
                                      "The object-type '" + type + "' "
                                      "is not supported. This is probably "
                                      "an oversight by the incapable "
                                      "programmers, please report this issue.");
        }
        if (!signalsBlocked())
                emit sceneChanged();
}



void RenderSettingsPropertyBrowser::variantManager_valueChanged(
        QtProperty*, QVariant const &
) {
        if (!signalsBlocked())
                emit sceneChanged();
}


void RenderSettingsPropertyBrowser::addRenderSettingsToScene (
        redshift::scenefile::Scene &scene
) const {
        typedef QList<QtProperty*> Props;
        typedef QtProperty* Prop;
        using namespace redshift;

        const Props renderSettings = readSubProperties("render-settings", root->properties());
        if (renderSettings.count()==0) {
                throw std::runtime_error("No Render-Setting present.");
        }

        foreach (QtProperty *mooh, renderSettingsProperty->subProperties()) {
                Props subs = mooh->subProperties();

                scenefile::RenderSettings rs;
                rs.title = readValueText("title", subs).toStdString();
                rs.width = readValue<unsigned int>("width", subs);
                rs.height = readValue<unsigned int>("height", subs);
                rs.samplesPerPixel = readValue<unsigned int>("samples-per-pixel", subs);
                rs.min_y = readValue<unsigned int>("min-y", subs);
                rs.max_y = readValue<unsigned int>("max-y", subs);
                rs.userSeed = readValue<unsigned int>("seed", subs);

                // Surface Integrator.
                Props si = readSubProperties("surface-integrator", subs);
                const QString sintegT = readValueText("type", si);
                if("none" == sintegT) {
                        rs.surfaceIntegrator.type = scenefile::SurfaceIntegrator::none;
                } else if("whitted" == sintegT) {
                        rs.surfaceIntegrator.type = scenefile::SurfaceIntegrator::whitted;
                } else if("whitted_ambient" == sintegT) {
                        rs.surfaceIntegrator.type = scenefile::SurfaceIntegrator::whitted_ambient;
                        rs.surfaceIntegrator.numAmbientSamples = readValue<int>("ambient-samples", si);
                } else if("path" == sintegT) {
                        rs.surfaceIntegrator.type = scenefile::SurfaceIntegrator::path;
                } else throw std::runtime_error((QString() + "The surface-integrator '" + sintegT + "' "
                                             "is not supported. This is probably "
                                             "an oversight by the incapable "
                                             "programmers, please report this issue.").toStdString().c_str());

                // Volume Integrator.
                Props vp = readSubProperties("volume-integrator", subs);
                rs.volumeIntegrator.stepSize = readValue<double>("step-size", vp);
                rs.volumeIntegrator.cutoffDistance = readValue<double>("cutoff-distance", vp);
                const QString integT = readValueText("type", vp);
                if ("none" == integT)
                        rs.volumeIntegrator.type = scenefile::VolumeIntegrator::none;
                else if ("emission" == integT)
                        rs.volumeIntegrator.type = scenefile::VolumeIntegrator::emission;
                else if ("single" == integT)
                        rs.volumeIntegrator.type = scenefile::VolumeIntegrator::single;
                else throw std::runtime_error((QString() +
                                              "The volume-integrator '" + integT + "' "
                                              "is not supported. This is probably "
                                              "an oversight by the incapable "
                                              "programmers, please report this issue.").toStdString().c_str());

                scene.addRenderSettings(rs);
        }
}



QStringList RenderSettingsPropertyBrowser::names () const {
        QStringList ret;
        foreach (QtProperty *mooh, renderSettingsProperty->subProperties())
                ret << readValueText("title", mooh->subProperties());
        return ret;
}



void RenderSettingsPropertyBrowser::enumManager_valueChanged (
        QtProperty*,
        int
) {
        if (!signalsBlocked())
                emit sceneChanged();
}
