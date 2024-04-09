#pragma once
const int NUMBER_CAMERAS = 2;

const int CAMERA_TOP_INDEX = 0;
const int CAMERA_BOT_INDEX = 1;

const int TAB_INDEX_LIVE_IMAGE = 0;
const int TAB_INDEX_ERROR_IMAGE = 1;

const int TAB_INDEX_MAIN_WIDGET_LIVE_IMAGE     = 0;
const int TAB_INDEX_MAIN_WIDGET_EDIT_REFERENCE = 1;
const int TAB_INDEX_MAIN_WIDGET_EDIT_PRODUCT   = 2;
const int TAB_INDEX_MAIN_WIDGET_CUSTOM_SETTINGS = 3;
const int TAB_INDEX_MAIN_WIDGET_GENERAL_SETTINGS = 4;
const int TAB_INDEX_MAIN_WIDGET_MEASURING_SETTINGS_TOP = 5;
const int TAB_INDEX_MAIN_WIDGET_MEASURING_SETTINGS_BOT = 6;


const int VIDEO_SOURCE_CAMERA = 0;
const int VIDEO_SOURCE_FILE = 1;

const int SAVE_FORMAT_IMAGE_NO_IMAGES        = 1;
const int SAVE_FORMAT_IMAGE_ALL_IMAGES       = 2;
const int SAVE_FORMAT_IMAGE_ONLY_BAD_IMAGES  = 3;
const int SAVE_FORMAT_IMAGE_ONLY_GOOD_IMAGES = 4;

const int NUMBER_ERROR_TYPES = 6;

const int ERROR_CODE_NO_ERROR = 0;
const int ERROR_CODE_FORMAT_NOT_FOUND_IN_SMALL_ROI = 2;
const int ERROR_CODE_FORMAT_NOT_FOUND_IN_BIG_ROI   = 3;
const int ERROR_CODE_FORMAT_NOT_FOUND_IN_FORMAT_ROI = 4;

const int ERROR_CODE_ANY_ERROR = 1000;
const int RETURN_CODE_REFERENCE_READY = 1001;

const int STATE_HOSE_UNDEFINED         = 0;
const int STATE_HOSE_NOT_FOUND         = 1;
const int STATE_HOSE_FOUND             = 2;
const int STATE_HOSE_START_POINT_FOUND = 4;
const int STATE_HOSE_END_POINT_FOUND   = 8;

const double SUPPORT_RECT_SIZE_IN_PIXEL = 12.0;

const int CURSER_POSITION_NOT_ON_ROI       = 0;
const int CURSER_POSITION_MOVE_TOP_LINE    = 1;
const int CURSER_POSITION_MOVE_BOTTOM_LINE = 2;
const int CURSER_POSITION_MOVE_LEFT_LINE   = 3;
const int CURSER_POSITION_MOVE_RIGHT_LINE  = 4;
const int CURSER_POSITION_RESIZE_TOP       = 5;
const int CURSER_POSITION_RESIZE_BOTTOM    = 6;
const int CURSER_POSITION_RESIZE_LEFT      = 7;
const int CURSER_POSITION_RESIZE_RIGHT     = 8;
const int CURSER_POSITION_MOVE_IN_ROI      = 9;
const int CURSER_POSITION_LEFT_LINE        = 10;
const int CURSER_POSITION_RIGHT_LINE       = 11;
const int CURSER_POSITION_TOP_LINE         = 12;
const int CURSER_POSITION_BOT_LINE         = 13;
const int CURSER_POSITION_MOVE_FULL_FORMAT = 14;


const int MINIMUM_ROI_SIZE_IN_PIXEL = 16;

const double RAD_PER_DEGREE = 3.14159265359 / 180.0;

const QString SHAPE_BASED_MODEL_FILE_NAME           = "ShapeBasedModelData.shm";
const QString SHAPE_BASED_MODEL_CONTOURE_FILE_NAME  = "ShapeBasedModelContoure.tiff";
const QString VARIATION_MODEL_FILE_NAME             = "VariationModelData.vam";
const QString VARIATION_MODEL_IMAGE_FILE_NAME       = "DiffRefImage.bmp";
const QString REFERENCE_MODEL_IMAGE_FILE_NAME       = "ReferenceModelImage.bmp";
const QString REFERENCE_IMAGE_FILE_NAME             = "ReferenceImage.bmp";

const QString MEASURING_PARAMETER_FILE_NAME         = "MeasuringParameter.ini";
const QString IMAGE_DATA_PARAMETER_FILE_NAME        = "ImageDataParameter.ini";

const QString SHARED_MEMORY_KEYNAME_FORMAT_IMAGE    = "FormatImage";
const QString SHARED_MEMORY_KEYNAME_FULL_HOSE_IMAGE = "FullHoseImage";

const QString MEASURE_TOOL_NAME_FORMAT_DETECTION    = "FormatInspection";
const QString MEASURE_TOOL_NAME_PRINT_CHECK         = "PrintCheck";
const QString MEASURE_TOOL_NAME_LINE_CHECK          = "LineCheck";

const QString VIDEO_FILE_EXTENSION                  = "png";

const QString DATE_TIME_FORMAT                      = "dd-MM-yy  HH:mm:ss";

const QString CAMERA_TOP_DIR_NAME                   = "CameraTop";
const QString CAMERA_BOT_DIR_NAME                   = "CameraBot";

const QString MODEL_NAME_FORMAT                     = "Format";

const unsigned long MAX_FORMAT_SIZE_IN_BYTES = 1048576 * 64;//80 MB

const int INSPECTION_ID_HOSE_WINDOW             = 100;
const int INSPECTION_ID_FORMAT_WINDOW           = 0;

const int STANDARD_Y_DIALOG_POSITION = 200;

const int HOSE_ERROR_TYPE_FONT_NOT_OK            = 1;
const int HOSE_ERROR_TYPE_POSITION_NOT_OK        = 2;
const int HOSE_ERROR_TYPE_LINE_MISSING           = 3;
const int HOSE_ERROR_TYPE_FONT_HEIGHT_OUT_OF_TOL = 4;
const int HOSE_ERROR_TYPE_FONT_WIDTH_OUT_OF_TOL  = 5;
const int HOSE_ERROR_TYPE_FORMAT_NOT_FOUND       = 6;

const int PLAY_VIDEO = 1;
const int STOP_VIDEO = 2;
const int SKIP_BACKWARD = 3;
const int SKIP_FORWARD = 4;

const int FILE_VERSION_BASE_NUMBER = 1000;
const int CURRENT_FILE_VERSION_NUMBER = FILE_VERSION_BASE_NUMBER + 2;



