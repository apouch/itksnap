#ifndef IMAGEIOWIZARDMODEL_H
#define IMAGEIOWIZARDMODEL_H

#include <string>

#include "AbstractModel.h"
#include "GuidedNativeImageIO.h"
#include "Registry.h"
#include "ImageIODelegates.h"
#include "ImageRegistrationManager.h"
#include "OptimizationProgressRenderer.h"

class GlobalUIModel;

namespace itk {
  class GDCMSeriesFileNames;
  class FastMutexLock;
}

/**
 This class provides a model for the ImageIO wizards. This allows the wizard
 to be distanced from the program logic. The wizard is just a collection of
 buttons and callbacks, but very little state

 This class is subclassed by specific dialogs, to allow customization. For
 example, save/load dialog behavior is handled this way.
 */
class ImageIOWizardModel : public AbstractModel
{
public:

  irisITKObjectMacro(ImageIOWizardModel, AbstractModel)

  itkEventMacro(RegistrationProgressEvent, IRISEvent)

  FIRES(RegistrationProgressEvent)

  typedef GuidedNativeImageIO::FileFormat FileFormat;
  enum Mode { LOAD, SAVE };

  enum SummaryItem {
    SI_FILENAME, SI_DIMS, SI_SPACING, SI_ORIGIN, SI_ORIENT,
    SI_ENDIAN, SI_COMPONENTS, SI_DATATYPE, SI_FILESIZE
  };

  // Initialize the wizard for load operations. Note that the delegate,
  // which specializes the behavior of this class, is stored internally
  // using a smart pointer, so its ownership can be relinquished to the
  // wizard.
  void InitializeForLoad(GlobalUIModel *parent,
                         AbstractLoadImageDelegate *delegate);

  void InitializeForSave(GlobalUIModel *parent,
                         AbstractSaveImageDelegate *delegate,
                         const char *dispName);

  irisGetMacro(Parent, GlobalUIModel *)
  irisGetMacro(GuidedIO, GuidedNativeImageIO *)

  // Does the model support loading
  bool IsLoadMode() const { return m_Mode == LOAD; }

  // Does the model support loading
  bool IsSaveMode() const { return m_Mode == SAVE; }

  /** Access the IO delegate used for loading */
  irisGetMacro(LoadDelegate, AbstractLoadImageDelegate *)

  /** Access the IO delegate used for saving */
  irisGetMacro(SaveDelegate, AbstractSaveImageDelegate *)

  // Whether we can save or load a file format
  virtual bool CanHandleFileFormat(FileFormat fmt);

  // This another method that checks if a loaded image is valid
  virtual bool CheckImageValidity();

  // Default extension for saving files
  virtual std::string GetDefaultFormatForSave() const
    { return std::string("NiFTI"); }

  /**
    Create a filter string for file IO dialogs. The lineEntry is in the
    printf format, with first %s being the title of the format, and the
    second being the list of extensions. extEntry is similar, used to print
    each extension. The examples for Qt are "%s (%s)" for lineEntry and
    "*.%s" for extEntry. For FLTK it would be "%s *.{%s}" for lineEntry
    and "%s" for extEntry. The separators are used to separate extensions
    per entry and rows in the filter string.
   */
  std::string GetFilter(const char *lineEntry,
                        const char *extEntry,
                        const char *extSeparator,
                        const char *rowSeparator);

  /**
    Guess the format for the file. In load mode, if the file does not exist,
    this will return FORMAT_COUNT, i.e., failure to determine format. If it
    exists, the format will be determined using registry information (if open
    before), magic number, and extension, in turn. If in save mode, format
    is detected using registry and extension only. The last parameter is only
    considered in Load mode.
    */
  FileFormat GuessFileFormat(const std::string &fname, bool &fileExists);

  /** Get the file format from file format name */
  FileFormat GetFileFormatByName(const std::string &formatName) const;

  /** Get the name of a file format */
  std::string GetFileFormatName(FileFormat fmt) const;

  /**
    Get the directory where to browse, given a currently entered file
    */
  std::string GetBrowseDirectory(const std::string &file);

  /**
    Get the size of the file in bytes
    */
  unsigned long GetFileSizeInBytes(const std::string &file);

  /**
    Get the history of filenames
    */
  irisGetMacro(HistoryName, std::string)

  /**
    Get the display name to show in the dialog
    */
  std::string GetDisplayName() const;

  /**
    * Reset the state of the model
    */
  void Reset();

  /**
    Set the format selected by the user
    */
  void SetSelectedFormat(FileFormat fmt);

  FileFormat GetSelectedFormat();

  /**
    Load the image from filename, putting warnings into a warning list. This
    may also fire an exception (e.g., if validation failed)
    */
  void LoadImage(std::string filename);

  /**
   Save the image to a filename
   */
  void SaveImage(std::string filename);

  /**
    Get the warnings generated by LoadImage
    */
  irisGetMacro(Warnings, IRISWarningList)

  /**
    Whether an image was loaded
    */
  bool IsImageLoaded() const;

  /**
    Get summary items to display for the user
    */
  std::string GetSummaryItem(SummaryItem item);

  /**
    Load relevant information from DICOM directory
    */
  void ProcessDicomDirectory(const std::string &filename);

  /**
    Get the DICOM directory contents
    */
  irisGetMacro(DicomContents, const GuidedNativeImageIO::RegistryArray &)

  /**
    Load n-th series from DICOM directory
    */
  void LoadDicomSeries(const std::string &filename, int series);


  irisGetSetMacro(SuggestedFilename, std::string)

  irisGetSetMacro(SuggestedFormat, GuidedNativeImageIO::FileFormat)

  /**
    Access the registry stored in the model and used for providing hints to
    the image IO.
    */
  Registry &GetHints()
    { return m_Registry; }

  /**
    Called just before exiting the wizard. Should update history, etc.
    */
  virtual void Finalize();

  /**
   * Set whether the IO module should use registration
   */
  irisGetMacro(UseRegistration, bool)

  /**
   * Is this an overlay?
   */
  irisIsMacro(Overlay)

  /** Should the overlay be loaded as sticky */
  irisSimplePropertyAccessMacro(StickyOverlay, bool)

  /** Which is the colormap of the sticky overlay */
  irisSimplePropertyAccessMacro(StickyOverlayColorMap, std::string)

  // Registration mode typedefs
  typedef ImageRegistrationManager::RegistrationMode RegistrationMode;
  typedef ImageRegistrationManager::RegistrationMetric RegistrationMetric;
  typedef ImageRegistrationManager::RegistrationInit RegistrationInit;

  // Registration domains
  typedef SimpleItemSetDomain<RegistrationMode, std::string> RegistrationModeDomain;
  typedef SimpleItemSetDomain<RegistrationMetric, std::string> RegistrationMetricDomain;
  typedef SimpleItemSetDomain<RegistrationInit, std::string> RegistrationInitDomain;

  // Access to registration models
  irisGenericPropertyAccessMacro(RegistrationMode, RegistrationMode, RegistrationModeDomain)
  irisGenericPropertyAccessMacro(RegistrationMetric, RegistrationMetric, RegistrationMetricDomain)
  irisGenericPropertyAccessMacro(RegistrationInit, RegistrationInit, RegistrationInitDomain)

  /**
   * Perform registration between loaded overlay and main image. This operation is meant to
   * be executed in a separate thread. From time to time, it will place the registration
   * results into a thread-safe variable and fire a progress event. Use the method
   * UpdateImageTransformFromRegistration() to apply registration results to the displayed image
   */
  void PerformRegistration();

  /**
   * Apply the currently computed transform to the image being loaded - allowing the user to
   * see the registration results on the fly
   */
  void UpdateImageTransformFromRegistration();

  /** Get the value of the registration objective function */
  double GetRegistrationObjective();

  /** Get the progress renderer object */
  irisGetMacro(RegistrationProgressRenderer, OptimizationProgressRenderer *)

protected:

  // Standard ITK protected constructors
  ImageIOWizardModel();
  virtual ~ImageIOWizardModel();

  // State of the model
  Mode m_Mode;

  // Delegate that does the actual loading or saving
  SmartPtr<AbstractLoadImageDelegate> m_LoadDelegate;

  // Delegate than handles saving
  SmartPtr<AbstractSaveImageDelegate> m_SaveDelegate;

  // The history list associated with the model
  std::string m_HistoryName, m_DisplayName;

  // Parent model
  GlobalUIModel *m_Parent;
  SmartPtr<GuidedNativeImageIO> m_GuidedIO;

  // Warnings generated during IO
  IRISWarningList m_Warnings;

  // Registry containing auxiliary info
  Registry m_Registry;

  // Whether the layer being loaded is an overlay
  bool m_Overlay;

  // Whether registration should be used to load this image
  bool m_UseRegistration;

  // Suggested filename
  std::string m_SuggestedFilename;

  // Suggested format
  GuidedNativeImageIO::FileFormat m_SuggestedFormat;

  // DICOM support
  GuidedNativeImageIO::RegistryArray m_DicomContents;

  // Overlay display behavior models
  SmartPtr<AbstractSimpleBooleanProperty> m_StickyOverlayModel;
  bool GetStickyOverlayValue(bool &value);
  void SetStickyOverlayValue(bool value);

  // Selected color map behavior model
  SmartPtr<AbstractSimpleStringProperty> m_StickyOverlayColorMapModel;
  bool GetStickyOverlayColorMapValue(std::string &value);
  void SetStickyOverlayColorMapValue(std::string value);

  // Registration models
  typedef ConcretePropertyModel<RegistrationMode, RegistrationModeDomain> RegistrationModeModel;
  typedef ConcretePropertyModel<RegistrationMetric, RegistrationMetricDomain> RegistrationMetricModel;
  typedef ConcretePropertyModel<RegistrationInit, RegistrationInitDomain> RegistrationInitModel;

  SmartPtr<RegistrationModeModel> m_RegistrationModeModel;
  SmartPtr<RegistrationMetricModel> m_RegistrationMetricModel;
  SmartPtr<RegistrationInitModel> m_RegistrationInitModel;

  // Registration manager
  SmartPtr<ImageRegistrationManager> m_RegistrationManager;

  // Renderer used to plot the metric
  SmartPtr<OptimizationProgressRenderer> m_RegistrationProgressRenderer;

  // Pointer to the image layer that has been loaded
  ImageWrapperBase *m_LoadedImage;
};

#endif // IMAGEIOWIZARDMODEL_H
