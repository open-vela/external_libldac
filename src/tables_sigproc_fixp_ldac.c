/*******************************************************************************
 *
 * Copyright (C) 2003 - 2021 Sony Corporation
 *
 ******************************************************************************/

#include "ldac.h"

/***************************************************************************************************
    Window Tables
***************************************************************************************************/
#ifndef _DECODE_ONLY
DECLFUNC const INT32 *gaa_fwin_ldac[LDAC_NUMLNN];
static const INT32 sa_fwin_1fs_ldac[LDAC_1FSLSU] = { /* Q30 */
    0x00009de9, 0x00058d10, 0x000f6a9a, 0x001e3503, 0x0031ea03, 0x004a868e, 0x006806db, 0x008a665c,
    0x00b19fc5, 0x00ddad09, 0x010e875c, 0x01442737, 0x017e8455, 0x01bd95b5, 0x0201519e, 0x0249ad9e,
    0x02969e8c, 0x02e8188c, 0x033e0f0c, 0x039874cb, 0x03f73bda, 0x045a5599, 0x04c1b2c1, 0x052d4362,
    0x059cf6e5, 0x0610bc11, 0x0688810b, 0x0704335c, 0x0783bff0, 0x0807131d, 0x088e18a1, 0x0918bbab,
    0x09a6e6da, 0x0a388442, 0x0acd7d6d, 0x0b65bb64, 0x0c0126ad, 0x0c9fa74f, 0x0d4124da, 0x0de58667,
    0x0e8cb29c, 0x0f368fb3, 0x0fe30379, 0x1091f357, 0x11434452, 0x11f6db14, 0x12ac9bea, 0x13646ace,
    0x141e2b67, 0x14d9c111, 0x15970edf, 0x1655f79f, 0x17165de0, 0x17d823f9, 0x189b2c07, 0x195f57f7,
    0x1a248988, 0x1aeaa254, 0x1bb183cc, 0x1c790f47, 0x1d4125ff, 0x1e09a918, 0x1ed279a9, 0x1f9b78b8,
    0x20648748, 0x212d8657, 0x21f656e8, 0x22beda01, 0x2386f0b9, 0x244e7c34, 0x25155dac, 0x25db7678,
    0x26a0a809, 0x2764d3f9, 0x2827dc07, 0x28e9a220, 0x29aa0861, 0x2a68f121, 0x2b263eef, 0x2be1d499,
    0x2c9b9532, 0x2d536416, 0x2e0924ec, 0x2ebcbbae, 0x2f6e0ca9, 0x301cfc87, 0x30c9704d, 0x31734d64,
    0x321a7999, 0x32bedb26, 0x336058b1, 0x33fed953, 0x349a449c, 0x35328293, 0x35c77bbe, 0x36591926,
    0x36e74455, 0x3771e75f, 0x37f8ece3, 0x387c4010, 0x38fbcca4, 0x39777ef5, 0x39ef43ef, 0x3a63091b,
    0x3ad2bc9e, 0x3b3e4d3f, 0x3ba5aa67, 0x3c08c426, 0x3c678b35, 0x3cc1f0f4, 0x3d17e774, 0x3d696174,
    0x3db65262, 0x3dfeae62, 0x3e426a4b, 0x3e817bab, 0x3ebbd8c9, 0x3ef178a4, 0x3f2252f7, 0x3f4e603b,
    0x3f7599a4, 0x3f97f925, 0x3fb57972, 0x3fce15fd, 0x3fe1cafd, 0x3ff09566, 0x3ffa72f0, 0x3fff6217,
};
static const INT32 sa_fwin_2fs_ldac[LDAC_2FSLSU] = { /* Q30 */
    0x0000277a, 0x0001634c, 0x0003dae2, 0x00078e25, 0x000c7cf0, 0x0012a713, 0x001a0c51, 0x0022ac60,
    0x002c86ec, 0x00379b93, 0x0043e9e8, 0x00517172, 0x006031aa, 0x00702a00, 0x008159d6, 0x0093c082,
    0x00a75d4f, 0x00bc2f7a, 0x00d23637, 0x00e970ac, 0x0101ddf4, 0x011b7d1e, 0x01364d2c, 0x01524d17,
    0x016f7bca, 0x018dd825, 0x01ad60fc, 0x01ce1518, 0x01eff336, 0x0212fa08, 0x02372835, 0x025c7c57,
    0x0282f4fd, 0x02aa90ad, 0x02d34ddf, 0x02fd2b01, 0x03282676, 0x03543e96, 0x038171ae, 0x03afbdff,
    0x03df21c0, 0x040f9b1f, 0x0441283b, 0x0473c72e, 0x04a77601, 0x04dc32b9, 0x0511fb4c, 0x0548cda7,
    0x0580a7ad, 0x05b98738, 0x05f36a15, 0x062e4e0a, 0x066a30d1, 0x06a7101b, 0x06e4e990, 0x0723bacd,
    0x07638166, 0x07a43ae5, 0x07e5e4cc, 0x08287c93, 0x086bffa9, 0x08b06b72, 0x08f5bd4d, 0x093bf28c,
    0x0983087b, 0x09cafc5d, 0x0a13cb6a, 0x0a5d72d6, 0x0aa7efc7, 0x0af33f61, 0x0b3f5eba, 0x0b8c4ae4,
    0x0bda00e8, 0x0c287dc7, 0x0c77be7a, 0x0cc7bff3, 0x0d187f1c, 0x0d69f8d9, 0x0dbc2a04, 0x0e0f0f74,
    0x0e62a5f6, 0x0eb6ea51, 0x0f0bd945, 0x0f616f8c, 0x0fb7a9d9, 0x100e84da, 0x1065fd35, 0x10be0f8b,
    0x1116b876, 0x116ff48c, 0x11c9c05b, 0x1224186e, 0x127ef94a, 0x12da5f6c, 0x1336474f, 0x1392ad69,
    0x13ef8e28, 0x144ce5f8, 0x14aab141, 0x1508ec64, 0x156793c0, 0x15c6a3ad, 0x16261883, 0x1685ee93,
    0x16e6222a, 0x1746af94, 0x17a79318, 0x1808c8f9, 0x186a4d78, 0x18cc1cd3, 0x192e3344, 0x19908d03,
    0x19f32646, 0x1a55fb3f, 0x1ab9081f, 0x1b1c4914, 0x1b7fba4b, 0x1be357ee, 0x1c471e27, 0x1cab091b,
    0x1d0f14f3, 0x1d733dd1, 0x1dd77fd9, 0x1e3bd72f, 0x1ea03ff3, 0x1f04b646, 0x1f69364a, 0x1fcdbc1e,
    0x203243e2, 0x2096c9b6, 0x20fb49ba, 0x215fc00d, 0x21c428d1, 0x22288027, 0x228cc22f, 0x22f0eb0d,
    0x2354f6e5, 0x23b8e1d9, 0x241ca812, 0x248045b5, 0x24e3b6ec, 0x2546f7e1, 0x25aa04c1, 0x260cd9ba,
    0x266f72fd, 0x26d1ccbc, 0x2733e32d, 0x2795b288, 0x27f73707, 0x28586ce8, 0x28b9506c, 0x2919ddd6,
    0x297a116d, 0x29d9e77d, 0x2a395c53, 0x2a986c40, 0x2af7139c, 0x2b554ebf, 0x2bb31a08, 0x2c1071d8,
    0x2c6d5297, 0x2cc9b8b1, 0x2d25a094, 0x2d8106b6, 0x2ddbe792, 0x2e363fa5, 0x2e900b74, 0x2ee9478a,
    0x2f41f075, 0x2f9a02cb, 0x2ff17b26, 0x30485627, 0x309e9074, 0x30f426bb, 0x314915af, 0x319d5a0a,
    0x31f0f08c, 0x3243d5fc, 0x32960727, 0x32e780e4, 0x3338400d, 0x33884186, 0x33d78239, 0x3425ff18,
    0x3473b51c, 0x34c0a146, 0x350cc09f, 0x35581039, 0x35a28d2a, 0x35ec3496, 0x363503a3, 0x367cf785,
    0x36c40d74, 0x370a42b3, 0x374f948e, 0x37940057, 0x37d7836d, 0x381a1b34, 0x385bc51b, 0x389c7e9a,
    0x38dc4533, 0x391b1670, 0x3958efe5, 0x3995cf2f, 0x39d1b1f6, 0x3a0c95eb, 0x3a4678c8, 0x3a7f5853,
    0x3ab73259, 0x3aee04b4, 0x3b23cd47, 0x3b5889ff, 0x3b8c38d2, 0x3bbed7c5, 0x3bf064e1, 0x3c20de40,
    0x3c504201, 0x3c7e8e52, 0x3cabc16a, 0x3cd7d98a, 0x3d02d4ff, 0x3d2cb221, 0x3d556f53, 0x3d7d0b03,
    0x3da383a9, 0x3dc8d7cb, 0x3ded05f8, 0x3e100cca, 0x3e31eae8, 0x3e529f04, 0x3e7227db, 0x3e908436,
    0x3eadb2e9, 0x3ec9b2d4, 0x3ee482e2, 0x3efe220c, 0x3f168f54, 0x3f2dc9c9, 0x3f43d086, 0x3f58a2b1,
    0x3f6c3f7e, 0x3f7ea62a, 0x3f8fd600, 0x3f9fce56, 0x3fae8e8e, 0x3fbc1618, 0x3fc8646d, 0x3fd37914,
    0x3fdd53a0, 0x3fe5f3af, 0x3fed58ed, 0x3ff38310, 0x3ff871db, 0x3ffc251e, 0x3ffe9cb4, 0x3fffd886,
};
#endif /* _DECODE_ONLY */
#ifndef _ENCODE_ONLY
DECLFUNC const INT32 *gaa_bwin_ldac[LDAC_NUMLNN];
static const INT32 sa_bwin_1fs_ldac[LDAC_1FSLSU] = { /* Q30 */
    0x00009dec, 0x00058e07, 0x000f720a, 0x001e5194, 0x0032381b, 0x004b34e9, 0x00695b2d, 0x008cc1ff,
    0x00b58467, 0x00e3c16f, 0x01179c29, 0x01513bbb, 0x0190cb70, 0x01d67abe, 0x02227d51, 0x02750b17,
    0x02ce6040, 0x032ebd44, 0x039666db, 0x0405a5f9, 0x047cc7b9, 0x04fc1d45, 0x0583fbaf, 0x0614bbbf,
    0x06aeb9b3, 0x075254e6, 0x07ffef6f, 0x08b7ed98, 0x097ab549, 0x0a48ad4d, 0x0b223c74, 0x0c07c897,
    0x0cf9b568, 0x0df86326, 0x0f042d0e, 0x101d67af, 0x11445efb, 0x1279542f, 0x13bc7b81, 0x150df9a0,
    0x166de10f, 0x17dc2f4e, 0x1958c9f2, 0x1ae37bab, 0x1c7bf14e, 0x1e21b6f5, 0x1fd4354a, 0x2192af18,
    0x235c3f4f, 0x252fd778, 0x270c3ee1, 0x28f01285, 0x2ad9c5d9, 0x2cc7a493, 0x2eb7d58a, 0x30a85ea5,
    0x329729f2, 0x34820bc2, 0x3666c9c4, 0x384322f7, 0x3a14d838, 0x3bd9b537, 0x3d8f998d, 0x3f3481a9,
    0x40c68f49, 0x4244112e, 0x43ab89d5, 0x44fbb4f8, 0x46338ba6, 0x475246ea, 0x485760e3, 0x4942945f,
    0x4a13db1b, 0x4acb6aaa, 0x4b69b05c, 0x4bef4c38, 0x4c5d0b51, 0x4cb3e1b0, 0x4cf4e403, 0x4d21414c,
    0x4d3a3cb1, 0x4d41279d, 0x4d375c4b, 0x4d1e38cb, 0x4cf71a95, 0x4cc35ab0, 0x4c844a6d, 0x4c3b30af,
    0x4be947c7, 0x4b8fbbbf, 0x4b2fa92c, 0x4aca1c5c, 0x4a6010dd, 0x49f2715d, 0x498217bf, 0x490fcd6b,
    0x489c4bc8, 0x48283cd9, 0x47b43bef, 0x4740d66f, 0x46ce8ca2, 0x465dd28e, 0x45ef10c9, 0x4582a557,
    0x4518e473, 0x44b21964, 0x444e8735, 0x43ee6975, 0x4391f4e7, 0x4339581f, 0x42e4bc22, 0x429444f2,
    0x4248120f, 0x42003ef6, 0x41bce38a, 0x417e147a, 0x4143e3a1, 0x410e6054, 0x40dd97b0, 0x40b194d9,
    0x408a6138, 0x406804af, 0x404a85c2, 0x4031e9c5, 0x401e34f6, 0x400f6a99, 0x40058d10, 0x40009de9,
};
static const INT32 sa_bwin_2fs_ldac[LDAC_2FSLSU] = { /* Q30 */
    0x0000277b, 0x0001635b, 0x0003db59, 0x00078fee, 0x000c81d1, 0x0012b1f6, 0x001a218d, 0x0022d206,
    0x002cc50c, 0x0037fc89, 0x00447aa3, 0x005241c1, 0x00615487, 0x0071b5d7, 0x008368d4, 0x009670de,
    0x00aad196, 0x00c08edd, 0x00d7acd5, 0x00f02fdf, 0x010a1c9f, 0x012577fa, 0x01424716, 0x01608f5d,
    0x01805679, 0x01a1a25b, 0x01c47933, 0x01e8e177, 0x020ee1e1, 0x0236816f, 0x025fc761, 0x028abb3e,
    0x02b764d1, 0x02e5cc28, 0x0315f997, 0x0347f5b4, 0x037bc95c, 0x03b17dad, 0x03e91c07, 0x0422ae0f,
    0x045e3dab, 0x049bd4ff, 0x04db7e71, 0x051d44a3, 0x05613274, 0x05a75300, 0x05efb197, 0x063a59c2,
    0x0687573e, 0x06d6b5f8, 0x07288209, 0x077cc7b6, 0x07d39369, 0x082cf1ac, 0x0888ef29, 0x08e7989e,
    0x0948fadb, 0x09ad22bc, 0x0a141d23, 0x0a7df6eb, 0x0aeabce6, 0x0b5a7bd4, 0x0bcd4056, 0x0c4316e6,
    0x0cbc0bce, 0x0d382b18, 0x0db78089, 0x0e3a178d, 0x0ebffb2e, 0x0f493604, 0x0fd5d223, 0x1065d912,
    0x10f953b3, 0x11904a33, 0x122ac3fb, 0x12c8c799, 0x136a5aae, 0x140f81d7, 0x14b8409c, 0x15649954,
    0x16148d14, 0x16c81b95, 0x177f431d, 0x183a0067, 0x18f84e8d, 0x19ba26ed, 0x1a7f8115, 0x1b4852a8,
    0x1c148f48, 0x1ce42881, 0x1db70db1, 0x1e8d2bf4, 0x1f666e12, 0x2042bc6a, 0x2121fce6, 0x220412e5,
    0x22e8df3a, 0x23d04018, 0x24ba110f, 0x25a62b0a, 0x2694644b, 0x2784906f, 0x28768074, 0x296a02c3,
    0x2a5ee33f, 0x2b54eb57, 0x2c4be21a, 0x2d438c59, 0x2e3bacbf, 0x2f3403fa, 0x302c50e2, 0x312450a7,
    0x321bbeff, 0x33125660, 0x3407d034, 0x34fbe51a, 0x35ee4d23, 0x36dec015, 0x37ccf5af, 0x38b8a5f3,
    0x39a18968, 0x3a87596b, 0x3b69d070, 0x3c48aa4f, 0x3d23a48c, 0x3dfa7e9b, 0x3eccfa25, 0x3f9adb4c,
    0x4063e8e4, 0x4127ecb1, 0x41e6b39c, 0x42a00de4, 0x4353cf4b, 0x4401cf3a, 0x44a9e8e9, 0x454bfb6f,
    0x45e7e9e3, 0x467d9b61, 0x470cfb1b, 0x4795f854, 0x48188664, 0x48949caa, 0x490a3681, 0x49795327,
    0x49e1f5ad, 0x4a4424d0, 0x4a9feae0, 0x4af55596, 0x4b4475ec, 0x4b8d5ff5, 0x4bd02aac, 0x4c0cefc6,
    0x4c43cb7f, 0x4c74dc6d, 0x4ca04347, 0x4cc622b6, 0x4ce69f21, 0x4d01de7b, 0x4d180810, 0x4d294458,
    0x4d35bcc2, 0x4d3d9b8d, 0x4d410b98, 0x4d403839, 0x4d3b4d16, 0x4d3275fe, 0x4d25dec6, 0x4d15b32a,
    0x4d021ea9, 0x4ceb4c6f, 0x4cd16736, 0x4cb49931, 0x4c950bf2, 0x4c72e85f, 0x4c4e5696, 0x4c277de8,
    0x4bfe84c3, 0x4bd390ac, 0x4ba6c636, 0x4b7848f4, 0x4b483b79, 0x4b16bf51, 0x4ae3f4fa, 0x4aaffbe8,
    0x4a7af27e, 0x4a44f611, 0x4a0e22ea, 0x49d69443, 0x499e644f, 0x4965ac38, 0x492c8429, 0x48f3034b,
    0x48b93fd2, 0x487f4efb, 0x48454517, 0x480b358d, 0x47d132e3, 0x47974ec3, 0x475d9a02, 0x472424a7,
    0x46eafdf1, 0x46b2345e, 0x4679d5b1, 0x4641eefc, 0x460a8ca2, 0x45d3ba61, 0x459d8357, 0x4567f20a,
    0x4533106c, 0x44fee7e3, 0x44cb814f, 0x4498e510, 0x44671b0a, 0x44362aae, 0x44061afc, 0x43d6f28a,
    0x43a8b788, 0x437b6fc9, 0x434f20c1, 0x4323cf90, 0x42f98102, 0x42d03995, 0x42a7fd7f, 0x4280d0ae,
    0x425ab6cf, 0x4235b351, 0x4211c965, 0x41eefc07, 0x41cd4dfe, 0x41acc1df, 0x418d5a10, 0x416f18cb,
    0x41520021, 0x413611fc, 0x411b5022, 0x4101bc34, 0x40e957b5, 0x40d22409, 0x40bc2278, 0x40a7542e,
    0x4093ba3e, 0x408155a5, 0x40702746, 0x40602ff3, 0x40517067, 0x4043e94e, 0x40379b3e, 0x402c86c1,
    0x4022ac4c, 0x401a0c48, 0x4012a710, 0x400c7cef, 0x40078e25, 0x4003dae2, 0x4001634c, 0x4000277a,
};
#endif /* _ENCODE_ONLY */

/***************************************************************************************************
    MDCT/IMDCT Tables
***************************************************************************************************/
DECLFUNC const INT32 *gaa_wcos_ldac[LDAC_NUMLNN];
static const INT32 sa_wcos_1fs_ldac[LDAC_1FSLSU] = { /* Q31 */
    0x5a82799a, 0x7641af3d, 0xcf043ab3, 0x7d8a5f40, 0x471cece7, 0xe70747c4, 0x9592675c, 0x7f62368f,
    0x70e2cbc6, 0x5133cc94, 0x25280c5e, 0xf3742ca2, 0xc3a94590, 0x9d0dfe54, 0x8582faa5, 0x7fd8878e,
    0x7c29fbee, 0x73b5ebd1, 0x66cf8120, 0x55f5a4d2, 0x41ce1e65, 0x2b1f34eb, 0x12c8106f, 0xf9b82684,
    0xe0e60685, 0xc945dfec, 0xb3c0200c, 0xa1288376, 0x9235f2ec, 0x877b7bec, 0x8162aa04, 0x7ff62182,
    0x7f0991c4, 0x7ce3ceb2, 0x798a23b1, 0x7504d345, 0x6f5f02b2, 0x68a69e81, 0x60ec3830, 0x5842dd54,
    0x4ebfe8a5, 0x447acd50, 0x398cdd32, 0x2e110a62, 0x2223a4c5, 0x15e21445, 0x096a9049, 0xfcdbd541,
    0xf054d8d5, 0xe3f47d96, 0xd7d946d8, 0xcc210d79, 0xc0e8b648, 0xb64beacd, 0xac64d510, 0xa34bdf20,
    0x9b1776da, 0x93dbd6a0, 0x8daad37b, 0x8893b125, 0x84a2fc62, 0x81e26c16, 0x8058c94c, 0x7fff6216,
    0x7ff09478, 0x7fce0c3e, 0x7f97cebd, 0x7f4de451, 0x7ef05860, 0x7e7f3957, 0x7dfa98a8, 0x7d628ac6,
    0x7cb72724, 0x7bf88830, 0x7b26cb4f, 0x7a4210d8, 0x794a7c12, 0x78403329, 0x77235f2d, 0x75f42c0b,
    0x74b2c884, 0x735f6626, 0x71fa3949, 0x708378ff, 0x6efb5f12, 0x6d6227fa, 0x6bb812d1, 0x69fd614a,
    0x683257ab, 0x66573cbb, 0x646c59bf, 0x6271fa69, 0x60686ccf, 0x5e50015d, 0x5c290acc, 0x59f3de12,
    0x57b0d256, 0x556040e2, 0x53028518, 0x5097fc5e, 0x4e210617, 0x4b9e0390, 0x490f57ee, 0x46756828,
    0x43d09aed, 0x4121589b, 0x3e680b2c, 0x3ba51e29, 0x38d8fe93, 0x36041ad9, 0x3326e2c3, 0x3041c761,
    0x2d553afc, 0x2a61b101, 0x27679df4, 0x24677758, 0x2161b3a0, 0x1e56ca1e, 0x1b4732ef, 0x183366e9,
    0x151bdf86, 0x120116d5, 0x0ee38766, 0x0bc3ac35, 0x08a2009a, 0x057f0035, 0x025b26d7, 0x00000000,
};
static const INT32 sa_wcos_2fs_ldac[LDAC_2FSLSU] = { /* Q31 */
    0x5a82799a, 0x7641af3d, 0xcf043ab3, 0x7d8a5f40, 0x471cece7, 0xe70747c4, 0x9592675c, 0x7f62368f,
    0x70e2cbc6, 0x5133cc94, 0x25280c5e, 0xf3742ca2, 0xc3a94590, 0x9d0dfe54, 0x8582faa5, 0x7fd8878e,
    0x7c29fbee, 0x73b5ebd1, 0x66cf8120, 0x55f5a4d2, 0x41ce1e65, 0x2b1f34eb, 0x12c8106f, 0xf9b82684,
    0xe0e60685, 0xc945dfec, 0xb3c0200c, 0xa1288376, 0x9235f2ec, 0x877b7bec, 0x8162aa04, 0x7ff62182,
    0x7f0991c4, 0x7ce3ceb2, 0x798a23b1, 0x7504d345, 0x6f5f02b2, 0x68a69e81, 0x60ec3830, 0x5842dd54,
    0x4ebfe8a5, 0x447acd50, 0x398cdd32, 0x2e110a62, 0x2223a4c5, 0x15e21445, 0x096a9049, 0xfcdbd541,
    0xf054d8d5, 0xe3f47d96, 0xd7d946d8, 0xcc210d79, 0xc0e8b648, 0xb64beacd, 0xac64d510, 0xa34bdf20,
    0x9b1776da, 0x93dbd6a0, 0x8daad37b, 0x8893b125, 0x84a2fc62, 0x81e26c16, 0x8058c94c, 0x7ffd885a,
    0x7fc25596, 0x7f3857f6, 0x7e5fe493, 0x7d3980ec, 0x7bc5e290, 0x7a05eead, 0x77fab989, 0x75a585cf,
    0x7307c3d0, 0x7023109a, 0x6cf934fc, 0x698c246c, 0x65ddfbd3, 0x61f1003f, 0x5dc79d7c, 0x59646498,
    0x54ca0a4b, 0x4ffb654d, 0x4afb6c98, 0x45cd358f, 0x4073f21d, 0x3af2eeb7, 0x354d9057, 0x2f875262,
    0x29a3c485, 0x23a6887f, 0x1d934fe5, 0x176dd9de, 0x1139f0cf, 0x0afb6805, 0x04b6195d, 0xfe6de2e0,
    0xf826a462, 0xf1e43d1c, 0xebaa894f, 0xe57d5fda, 0xdf608fe4, 0xd957de7a, 0xd3670446, 0xcd91ab39,
    0xc7db6c50, 0xc247cd5a, 0xbcda3ecb, 0xb796199b, 0xb27e9d3c, 0xad96ed92, 0xa8e21106, 0xa462eeac,
    0xa01c4c73, 0x9c10cd70, 0x9842f043, 0x94b50d87, 0x91695663, 0x8e61d32e, 0x8ba0622f, 0x8926b677,
    0x86f656d3, 0x85109cdd, 0x8376b422, 0x82299971, 0x812a1a3a, 0x8078d40d, 0x80163440, 0x7fffd886,
    0x7ffc250f, 0x7ff38274, 0x7fe5f108, 0x7fd37153, 0x7fbc040a, 0x7f9faa15, 0x7f7e648c, 0x7f5834b7,
    0x7f2d1c0e, 0x7efd1c3c, 0x7ec8371a, 0x7e8e6eb2, 0x7e4fc53e, 0x7e0c3d29, 0x7dc3d90d, 0x7d769bb5,
    0x7d24881b, 0x7ccda169, 0x7c71eaf9, 0x7c116853, 0x7bac1d31, 0x7b420d7a, 0x7ad33d45, 0x7a5fb0d8,
    0x79e76ca7, 0x796a7554, 0x78e8cfb2, 0x786280bf, 0x77d78daa, 0x7747fbce, 0x76b3d0b4, 0x761b1211,
    0x757dc5ca, 0x74dbf1ef, 0x74359cbd, 0x738acc9e, 0x72db8828, 0x7227d61c, 0x716fbd68, 0x70b34525,
    0x6ff27497, 0x6f2d532c, 0x6e63e87f, 0x6d963c54, 0x6cc45698, 0x6bee3f62, 0x6b13fef5, 0x6a359db9,
    0x69532442, 0x686c9b4b, 0x67820bb7, 0x66937e91, 0x65a0fd0b, 0x64aa907f, 0x63b0426d, 0x62b21c7b,
    0x61b02876, 0x60aa7050, 0x5fa0fe1f, 0x5e93dc1f, 0x5d8314b1, 0x5c6eb258, 0x5b56bfbd, 0x5a3b47ab,
    0x591c550e, 0x57f9f2f8, 0x56d42c99, 0x55ab0d46, 0x547ea073, 0x534ef1b5, 0x521c0cc2, 0x50e5fd6d,
    0x4faccfab, 0x4e708f8f, 0x4d31494b, 0x4bef092d, 0x4aa9dba2, 0x4961cd33, 0x4816ea86, 0x46c9405c,
    0x4578db93, 0x4425c923, 0x42d0161e, 0x4177cfb1, 0x401d0321, 0x3ebfbdcd, 0x3d600d2c, 0x3bfdfecd,
    0x3a99a057, 0x3932ff87, 0x37ca2a30, 0x365f2e3b, 0x34f219a8, 0x3382fa88, 0x3211df04, 0x309ed556,
    0x2f29ebcc, 0x2db330c7, 0x2c3ab2b9, 0x2ac08026, 0x2944a7a2, 0x27c737d3, 0x26483f6c, 0x24c7cd33,
    0x2345eff8, 0x21c2b69c, 0x203e300d, 0x1eb86b46, 0x1d31774d, 0x1ba96335, 0x1a203e1b, 0x18961728,
    0x170afd8d, 0x157f0086, 0x13f22f58, 0x1264994e, 0x10d64dbd, 0x0f475bff, 0x0db7d376, 0x0c27c389,
    0x0a973ba5, 0x09064b3a, 0x077501be, 0x05e36ea9, 0x0451a177, 0x02bfa9a4, 0x012d96b1, 0x00000000,
};

DECLFUNC const INT32 *gaa_wsin_ldac[LDAC_NUMLNN];
static const INT32 sa_wsin_1fs_ldac[LDAC_1FSLSU] = { /* Q31 */
    0x5a82799a, 0x30fbc54d, 0x7641af3d, 0x18f8b83c, 0x6a6d98a4, 0x7d8a5f40, 0x471cece7, 0x0c8bd35e,
    0x3c56ba70, 0x62f201ac, 0x7a7d055b, 0x7f62368f, 0x70e2cbc6, 0x5133cc94, 0x25280c5e, 0x0647d97c,
    0x1f19f97b, 0x36ba2014, 0x4c3fdff4, 0x5ed77c8a, 0x6dca0d14, 0x78848414, 0x7e9d55fc, 0x7fd8878e,
    0x7c29fbee, 0x73b5ebd1, 0x66cf8120, 0x55f5a4d2, 0x41ce1e65, 0x2b1f34eb, 0x12c8106f, 0x03242abf,
    0x0fab272b, 0x1c0b826a, 0x2826b928, 0x33def287, 0x3f1749b8, 0x49b41533, 0x539b2af0, 0x5cb420e0,
    0x64e88926, 0x6c242960, 0x72552c85, 0x776c4edb, 0x7b5d039e, 0x7e1d93ea, 0x7fa736b4, 0x7ff62182,
    0x7f0991c4, 0x7ce3ceb2, 0x798a23b1, 0x7504d345, 0x6f5f02b2, 0x68a69e81, 0x60ec3830, 0x5842dd54,
    0x4ebfe8a5, 0x447acd50, 0x398cdd32, 0x2e110a62, 0x2223a4c5, 0x15e21445, 0x096a9049, 0x00c90f88,
    0x03ed26e6, 0x0710a345, 0x0a3308bd, 0x0d53db92, 0x1072a048, 0x138edbb1, 0x16a81305, 0x19bdcbf3,
    0x1ccf8cb3, 0x1fdcdc1b, 0x22e541af, 0x25e845b6, 0x28e5714b, 0x2bdc4e6f, 0x2ecc681e, 0x31b54a5e,
    0x34968250, 0x376f9e46, 0x3a402dd2, 0x3d07c1d6, 0x3fc5ec98, 0x427a41d0, 0x452456bd, 0x47c3c22f,
    0x4a581c9e, 0x4ce10034, 0x4f5e08e3, 0x51ced46e, 0x5433027d, 0x568a34a9, 0x58d40e8c, 0x5b1035cf,
    0x5d3e5237, 0x5f5e0db3, 0x616f146c, 0x637114cc, 0x6563bf92, 0x6746c7d8, 0x6919e320, 0x6adcc964,
    0x6c8f351c, 0x6e30e34a, 0x6fc19385, 0x71410805, 0x72af05a7, 0x740b53fb, 0x7555bd4c, 0x768e0ea6,
    0x77b417df, 0x78c7aba2, 0x79c89f6e, 0x7ab6cba4, 0x7b920b89, 0x7c5a3d50, 0x7d0f4218, 0x7db0fdf8,
    0x7e3f57ff, 0x7eba3a39, 0x7f2191b4, 0x7f754e80, 0x7fb563b3, 0x7fe1c76b, 0x7ffa72d1, 0x00000000,
};
static const INT32 sa_wsin_2fs_ldac[LDAC_2FSLSU] = { /* Q31 */
    0x5a82799a, 0x30fbc54d, 0x7641af3d, 0x18f8b83c, 0x6a6d98a4, 0x7d8a5f40, 0x471cece7, 0x0c8bd35e,
    0x3c56ba70, 0x62f201ac, 0x7a7d055b, 0x7f62368f, 0x70e2cbc6, 0x5133cc94, 0x25280c5e, 0x0647d97c,
    0x1f19f97b, 0x36ba2014, 0x4c3fdff4, 0x5ed77c8a, 0x6dca0d14, 0x78848414, 0x7e9d55fc, 0x7fd8878e,
    0x7c29fbee, 0x73b5ebd1, 0x66cf8120, 0x55f5a4d2, 0x41ce1e65, 0x2b1f34eb, 0x12c8106f, 0x03242abf,
    0x0fab272b, 0x1c0b826a, 0x2826b928, 0x33def287, 0x3f1749b8, 0x49b41533, 0x539b2af0, 0x5cb420e0,
    0x64e88926, 0x6c242960, 0x72552c85, 0x776c4edb, 0x7b5d039e, 0x7e1d93ea, 0x7fa736b4, 0x7ff62182,
    0x7f0991c4, 0x7ce3ceb2, 0x798a23b1, 0x7504d345, 0x6f5f02b2, 0x68a69e81, 0x60ec3830, 0x5842dd54,
    0x4ebfe8a5, 0x447acd50, 0x398cdd32, 0x2e110a62, 0x2223a4c5, 0x15e21445, 0x096a9049, 0x01921d20,
    0x07d95b9e, 0x0e1bc2e4, 0x145576b1, 0x1a82a026, 0x209f701c, 0x26a82186, 0x2c98fbba, 0x326e54c7,
    0x382493b0, 0x3db832a6, 0x4325c135, 0x4869e665, 0x4d8162c4, 0x5269126e, 0x571deefa, 0x5b9d1154,
    0x5fe3b38d, 0x63ef3290, 0x67bd0fbd, 0x6b4af279, 0x6e96a99d, 0x719e2cd2, 0x745f9dd1, 0x76d94989,
    0x7909a92d, 0x7aef6323, 0x7c894bde, 0x7dd6668f, 0x7ed5e5c6, 0x7f872bf3, 0x7fe9cbc0, 0x7ffd885a,
    0x7fc25596, 0x7f3857f6, 0x7e5fe493, 0x7d3980ec, 0x7bc5e290, 0x7a05eead, 0x77fab989, 0x75a585cf,
    0x7307c3d0, 0x7023109a, 0x6cf934fc, 0x698c246c, 0x65ddfbd3, 0x61f1003f, 0x5dc79d7c, 0x59646498,
    0x54ca0a4b, 0x4ffb654d, 0x4afb6c98, 0x45cd358f, 0x4073f21d, 0x3af2eeb7, 0x354d9057, 0x2f875262,
    0x29a3c485, 0x23a6887f, 0x1d934fe5, 0x176dd9de, 0x1139f0cf, 0x0afb6805, 0x04b6195d, 0x006487e3,
    0x01f6a297, 0x0388a9ea, 0x051a8e5c, 0x06ac406f, 0x083db0a7, 0x09cecf89, 0x0b5f8d9f, 0x0cefdb76,
    0x0e7fa99e, 0x100ee8ad, 0x119d8941, 0x132b7bf9, 0x14b8b17f, 0x16451a83, 0x17d0a7bc, 0x195b49ea,
    0x1ae4f1d6, 0x1c6d9053, 0x1df5163f, 0x1f7b7481, 0x21009c0c, 0x22847de0, 0x24070b08, 0x2588349d,
    0x2707ebc7, 0x288621b9, 0x2a02c7b8, 0x2b7dcf17, 0x2cf72939, 0x2e6ec792, 0x2fe49ba7, 0x3158970e,
    0x32caab6f, 0x343aca87, 0x35a8e625, 0x3714f02a, 0x387eda8e, 0x39e6975e, 0x3b4c18ba, 0x3caf50da,
    0x3e10320d, 0x3f6eaeb8, 0x40cab958, 0x42244481, 0x437b42e1, 0x44cfa740, 0x4621647d, 0x47706d93,
    0x48bcb599, 0x4a062fbd, 0x4b4ccf4d, 0x4c9087b1, 0x4dd14c6e, 0x4f0f1126, 0x5049c999, 0x518169a5,
    0x52b5e546, 0x53e73097, 0x55153fd4, 0x56400758, 0x57677b9d, 0x588b9140, 0x59ac3cfd, 0x5ac973b5,
    0x5be32a67, 0x5cf95638, 0x5e0bec6e, 0x5f1ae274, 0x60262dd6, 0x612dc447, 0x62319b9d, 0x6331a9d4,
    0x642de50d, 0x6526438f, 0x661abbc5, 0x670b4444, 0x67f7d3c5, 0x68e06129, 0x69c4e37a, 0x6aa551e9,
    0x6b81a3cd, 0x6c59d0a9, 0x6d2dd027, 0x6dfd9a1c, 0x6ec92683, 0x6f906d84, 0x70536771, 0x71120cc5,
    0x71cc5626, 0x72823c67, 0x7333b883, 0x73e0c3a3, 0x7489571c, 0x752d6c6c, 0x75ccfd42, 0x76680376,
    0x76fe790e, 0x7790583e, 0x781d9b65, 0x78a63d11, 0x792a37fe, 0x79a98715, 0x7a24256f, 0x7a9a0e50,
    0x7b0b3d2c, 0x7b77ada8, 0x7bdf5b94, 0x7c4242f2, 0x7ca05ff1, 0x7cf9aef0, 0x7d4e2c7f, 0x7d9dd55a,
    0x7de8a670, 0x7e2e9cdf, 0x7e6fb5f4, 0x7eabef2c, 0x7ee34636, 0x7f15b8ee, 0x7f434563, 0x7f6be9d4,
    0x7f8fa4b0, 0x7fae7495, 0x7fc85854, 0x7fdd4eec, 0x7fed5791, 0x7ff871a2, 0x7ffe9cb2, 0x00000000,
};

DECLFUNC const int *gaa_perm_ldac[LDAC_NUMLNN];
static const int sa_perm_1fs_ldac[LDAC_1FSLSU] = {
      0,  64,  96,  32,  48, 112,  80,  16,  24,  88, 120,  56,  40, 104,  72,   8,
     12,  76, 108,  44,  60, 124,  92,  28,  20,  84, 116,  52,  36, 100,  68,   4,
      6,  70, 102,  38,  54, 118,  86,  22,  30,  94, 126,  62,  46, 110,  78,  14,
     10,  74, 106,  42,  58, 122,  90,  26,  18,  82, 114,  50,  34,  98,  66,   2,
      3,  67,  99,  35,  51, 115,  83,  19,  27,  91, 123,  59,  43, 107,  75,  11,
     15,  79, 111,  47,  63, 127,  95,  31,  23,  87, 119,  55,  39, 103,  71,   7,
      5,  69, 101,  37,  53, 117,  85,  21,  29,  93, 125,  61,  45, 109,  77,  13,
      9,  73, 105,  41,  57, 121,  89,  25,  17,  81, 113,  49,  33,  97,  65,   1,
};
static const int sa_perm_2fs_ldac[LDAC_2FSLSU] = {
      0, 128, 192,  64,  96, 224, 160,  32,  48, 176, 240, 112,  80, 208, 144,  16,
     24, 152, 216,  88, 120, 248, 184,  56,  40, 168, 232, 104,  72, 200, 136,   8,
     12, 140, 204,  76, 108, 236, 172,  44,  60, 188, 252, 124,  92, 220, 156,  28,
     20, 148, 212,  84, 116, 244, 180,  52,  36, 164, 228, 100,  68, 196, 132,   4,
      6, 134, 198,  70, 102, 230, 166,  38,  54, 182, 246, 118,  86, 214, 150,  22,
     30, 158, 222,  94, 126, 254, 190,  62,  46, 174, 238, 110,  78, 206, 142,  14,
     10, 138, 202,  74, 106, 234, 170,  42,  58, 186, 250, 122,  90, 218, 154,  26,
     18, 146, 210,  82, 114, 242, 178,  50,  34, 162, 226,  98,  66, 194, 130,   2,
      3, 131, 195,  67,  99, 227, 163,  35,  51, 179, 243, 115,  83, 211, 147,  19,
     27, 155, 219,  91, 123, 251, 187,  59,  43, 171, 235, 107,  75, 203, 139,  11,
     15, 143, 207,  79, 111, 239, 175,  47,  63, 191, 255, 127,  95, 223, 159,  31,
     23, 151, 215,  87, 119, 247, 183,  55,  39, 167, 231, 103,  71, 199, 135,   7,
      5, 133, 197,  69, 101, 229, 165,  37,  53, 181, 245, 117,  85, 213, 149,  21,
     29, 157, 221,  93, 125, 253, 189,  61,  45, 173, 237, 109,  77, 205, 141,  13,
      9, 137, 201,  73, 105, 233, 169,  41,  57, 185, 249, 121,  89, 217, 153,  25,
     17, 145, 209,  81, 113, 241, 177,  49,  33, 161, 225,  97,  65, 193, 129,   1,
};

/***************************************************************************************************
    Normalization Tables
***************************************************************************************************/
#ifndef _DECODE_ONLY
/* Scale Factor for Spectrum Normalization */
DECLFUNC const INT32 ga_sf_ldac[LDAC_NIDSF] = { /* Q15 */
    0x00000001, 0x00000002, 0x00000004, 0x00000008,
    0x00000010, 0x00000020, 0x00000040, 0x00000080,
    0x00000100, 0x00000200, 0x00000400, 0x00000800,
    0x00001000, 0x00002000, 0x00004000, 0x00008000,
    0x00010000, 0x00020000, 0x00040000, 0x00080000,
    0x00100000, 0x00200000, 0x00400000, 0x00800000,
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x7fffffff,
};
#endif /* _DECODE_ONLY */

/***************************************************************************************************
    Quantization Tables
***************************************************************************************************/
#ifndef _DECODE_ONLY
/* Quantize Factor for Spectrum/Residual Quantization */
DECLFUNC const INT32 ga_qf_ldac[LDAC_NIDWL] = { /* Q16 */
    0x00008000, 0x00018000, 0x00038000, 0x00078000,
    0x000f8000, 0x001f8000, 0x003f8000, 0x007f8000,
    0x00ff8000, 0x01ff8000, 0x03ff8000, 0x07ff8000,
    0x0fff8000, 0x1fff8000, 0x3fff8000, 0x7fff8000,
};
#endif /* _DECODE_ONLY */

/* Inverse of Quantize Factor for Spectrum/Residual Quantization */
DECLFUNC const INT32 ga_iqf_ldac[LDAC_NIDWL] = { /* Q31 */
    0x80000000, 0x55555555, 0x24924925, 0x11111111,
    0x08421084, 0x04104104, 0x02040810, 0x01010101,
    0x00804020, 0x00401004, 0x00200401, 0x00100100,
    0x00080040, 0x00040010, 0x00020004, 0x00010001,
};

#ifndef _DECODE_ONLY
/* Inverse of Scale Factor for Residual Normalization */
DECLFUNC const INT32 ga_irsf_ldac[LDAC_NIDWL] = { /* Q15 */
    0x00007f80, 0x00017e80, 0x00037c80, 0x00077880,
    0x000f7080, 0x001f6080, 0x003f4080, 0x007f0080,
    0x00fe8080, 0x01fd8080, 0x03fb8080, 0x07f78080,
    0x0fef8080, 0x1fdf8080, 0x3fbf8080, 0x7f7f8080,
};
#endif /* _DECODE_ONLY */

#ifndef _ENCODE_ONLY
/* Scale Factor for Residual Normalization */
DECLFUNC const INT32 ga_rsf_ldac[LDAC_NIDWL] = { /* Q31 */
    0x80000000, 0x2aaaaaab, 0x12492492, 0x08888889,
    0x04210842, 0x02082082, 0x01020408, 0x00808081,
    0x00402010, 0x00200802, 0x00100200, 0x00080080,
    0x00040020, 0x00020008, 0x00010002, 0x00008001,
};
#endif /* _ENCODE_ONLY */

#ifndef _DECODE_ONLY
/***************************************************************************************************
    Set MDCT Tables
***************************************************************************************************/
DECLFUNC void set_mdct_table_ldac(
int nlnn)
{
    int index = nlnn - LDAC_1FSLNN;

    if (nlnn == LDAC_1FSLNN) {
        gaa_fwin_ldac[index] = sa_fwin_1fs_ldac;
        gaa_wcos_ldac[index] = sa_wcos_1fs_ldac;
        gaa_wsin_ldac[index] = sa_wsin_1fs_ldac;
        gaa_perm_ldac[index] = sa_perm_1fs_ldac;
    }
    else if (nlnn == LDAC_2FSLNN) {
        gaa_fwin_ldac[index] = sa_fwin_2fs_ldac;
        gaa_wcos_ldac[index] = sa_wcos_2fs_ldac;
        gaa_wsin_ldac[index] = sa_wsin_2fs_ldac;
        gaa_perm_ldac[index] = sa_perm_2fs_ldac;
    }

    return;
}
#endif /* _DECODE_ONLY */

#ifndef _ENCODE_ONLY
/***************************************************************************************************
    Set IMDCT Tables
***************************************************************************************************/
DECLFUNC void set_imdct_table_ldac(
int nlnn)
{
    int index = nlnn - LDAC_1FSLNN;

    if (nlnn == LDAC_1FSLNN) {
        gaa_bwin_ldac[index] = sa_bwin_1fs_ldac;
        gaa_wcos_ldac[index] = sa_wcos_1fs_ldac;
        gaa_wsin_ldac[index] = sa_wsin_1fs_ldac;
        gaa_perm_ldac[index] = sa_perm_1fs_ldac;
    }
    else if (nlnn == LDAC_2FSLNN) {
        gaa_bwin_ldac[index] = sa_bwin_2fs_ldac;
        gaa_wcos_ldac[index] = sa_wcos_2fs_ldac;
        gaa_wsin_ldac[index] = sa_wsin_2fs_ldac;
        gaa_perm_ldac[index] = sa_perm_2fs_ldac;
    }

    return;
}
#endif /* _ENCODE_ONLY */

