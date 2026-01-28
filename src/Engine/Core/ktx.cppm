module;

#include <ktx.h>

export module Engine.Core.KTX;

export
{
  using ::ktxTexture;
  using ::ktxTexture2;
  using ::ktx_error_code_e;
  using ::ktxResult;
  using ::ktx_transcode_fmt_e;
  using ::ktx_size_t;

  using ::ktxTexture_CreateFromNamedFile;
  using ::ktxTexture2_NeedsTranscoding;
  using ::ktxTexture2_TranscodeBasis;
  using ::ktxTexture2_GetImageOffset;
  using ::ktxErrorString;

  using ::KTX_SUCCESS;

  using ::KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT;

  using ::KTX_TTF_BC1_RGB;
  using ::KTX_TTF_BC4_R;
  using ::KTX_TTF_BC5_RG;
  using ::KTX_TTF_BC7_RGBA;
}
