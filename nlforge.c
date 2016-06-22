#include <ncurses.h>

static char data[] = {
  0xdb, 0x45, 0xfa, 0x34, 0x52, 0x86, 0x2f, 0x5b, 0x31, 0xec, 0x91, 0x92, 0xfc, 0x8a, 0x14, 0x8d,
  0xb7, 0xf3, 0xa8, 0xe7, 0xd4, 0x09, 0xbf, 0x05, 0xf9, 0x36, 0xd7, 0x8d, 0xe5, 0x5f, 0x06, 0x71,
  0xf0, 0x97, 0x8c, 0x0f, 0xab, 0x0f, 0x1a, 0x80, 0x01, 0x45, 0xa5, 0x07, 0x25, 0x85, 0xe2, 0x6d,
  0xb6, 0xe6, 0x99, 0x07, 0x5b, 0xa6, 0x65, 0x7d, 0xb3, 0xea, 0xb8, 0x3b, 0xd4, 0xe1, 0xc8, 0x45,
  0xd4, 0xf8, 0xc1, 0x89, 0x86, 0x50, 0xef, 0xc7, 0x05, 0x1a, 0x16, 0x8a, 0xc6, 0x5f, 0xcf, 0x0b,
  0xcd, 0x8c, 0x67, 0xe7, 0x40, 0x6b, 0x98, 0x27, 0xaa, 0x8f, 0x3d, 0x7f, 0x65, 0x67, 0xc9, 0xcf,
  0xa2, 0xa2, 0xb7, 0xb5, 0xd8, 0xde, 0x24, 0xb9, 0x74, 0xf6, 0xb4, 0xcf, 0x75, 0x52, 0x60, 0x33,
  0xe8, 0xda, 0x1d, 0xb0, 0x06, 0x18, 0xb1, 0x54, 0x67, 0x60, 0x47, 0x9e, 0xeb, 0xa2, 0x68, 0xe7,
  0x71, 0x5c, 0x82, 0xd6, 0xa8, 0xac, 0x7c, 0x93, 0x79, 0xcf, 0x2d, 0x7e, 0x16, 0xf1, 0x8f, 0x89,
  0xd6, 0x43, 0xbe, 0x61, 0x16, 0x35, 0xd8, 0x6a, 0xd0, 0xab, 0x6f, 0x82, 0x31, 0x0b, 0xd9, 0x0e,
  0xf8, 0x81, 0x8f, 0x38, 0xe2, 0xdd, 0x1f, 0x32, 0xa8, 0x1a, 0xe8, 0xab, 0x38, 0xd7, 0x56, 0x5f,
  0x99, 0x6c, 0x97, 0x30, 0x3f, 0xab, 0x5a, 0xc1, 0x21, 0x47, 0x34, 0x40, 0x52, 0x4c, 0xb3, 0xb0,
  0xb4, 0x74, 0xc6, 0x1d, 0x62, 0xba, 0xbf, 0xf8, 0x99, 0xfb, 0xbb, 0x1f, 0x66, 0x12, 0x65, 0x14,
  0xf0, 0x04, 0x44, 0x37, 0xdd, 0x79, 0x31, 0xab, 0x7b, 0x2f, 0xb1, 0x73, 0x2f, 0xf7, 0x7b, 0xfe,
  0xbb, 0x16, 0x95, 0xe1, 0x4d, 0x43, 0xc9, 0xf8, 0x8a, 0x74, 0x18, 0x53, 0xd0, 0x82, 0x13, 0xe5,
  0x74, 0x1b, 0xa5, 0x14, 0x72, 0xe9, 0xe1, 0x87, 0xd7, 0xd9, 0xda, 0x89, 0xa2, 0x3a, 0x12, 0xc3,
  0xfc, 0x7c, 0x0d, 0x60, 0x91, 0x09, 0xc5, 0xf8, 0xf5, 0xed, 0xba, 0x59, 0x32, 0x19, 0x0d, 0xbe,
  0xb7, 0xdd, 0xf8, 0x72, 0x6f, 0x25, 0x9e, 0x56, 0x31, 0xe9, 0xe8, 0xa6, 0x4f, 0xbd, 0x62, 0x69,
  0xcf, 0xa0, 0x59, 0x77, 0x14, 0xbe, 0x0a, 0xa7, 0xb5, 0x7c, 0xdd, 0xc1, 0xcb, 0xd8, 0x14, 0x70,
  0xb1, 0xf1, 0xb9, 0x7e, 0x95, 0x80, 0x37, 0xf9, 0x2d, 0xf2, 0xb0, 0x32, 0x71, 0xd9, 0xce, 0x29,
  0xef, 0xe1, 0x6a, 0x3d, 0x94, 0x1e, 0xc3, 0xb3, 0x2a, 0x85, 0xb1, 0xff, 0xa8, 0x16, 0xf9, 0xce,
  0x56, 0x23, 0x7b, 0xfc, 0x2c, 0x59, 0x5d, 0xab, 0x22, 0xd9, 0x65, 0x71, 0x4f, 0x3a, 0x7c, 0xb4,
  0x7b, 0x7d, 0xec, 0xd4, 0x13, 0xc4, 0xea, 0x9c, 0x55, 0x03, 0x10, 0xe2, 0xf2, 0x4f, 0x31, 0x6f,
  0x73, 0xb3, 0xbd, 0x79, 0xbd, 0x45, 0x3a, 0xa5, 0xad, 0xa9, 0x87, 0xcf, 0x86, 0x5f, 0xac, 0x25,
  0x23, 0x3c, 0x59, 0x3c, 0x56, 0xd9, 0x82, 0x2f, 0xc3, 0x41, 0x92, 0x26, 0xa7, 0xb6, 0x2b, 0xba,
  0x30, 0xa7, 0xf7, 0xc6, 0xa4, 0xe9, 0x17, 0xa0, 0xbc, 0xb4, 0x8e, 0x6a, 0x69, 0xca, 0xf6, 0xe5,
  0x0f, 0xd1, 0x52, 0xe1, 0xa5, 0x13, 0xc7, 0xcc, 0xf7, 0x1c, 0x3b, 0xb4, 0xdb, 0xa2, 0x16, 0x7c,
  0x34, 0x9f, 0x26, 0xe2, 0xd7, 0xac, 0xd1, 0xaf, 0xf6, 0xbd, 0x19, 0x2b, 0x28, 0xdc, 0x5b, 0xfc,
  0x24, 0xf6, 0x16, 0x3f, 0xc5, 0xd3, 0x37, 0xcb, 0x01, 0x2e, 0x9a, 0xa2, 0x96, 0x3c, 0x98, 0xc9,
  0x98, 0x8b, 0x87, 0x24, 0x62, 0x7b, 0xb4, 0x26, 0x53, 0x6c, 0xde, 0xf1, 0x69, 0xff, 0x11, 0x1e,
  0xa6, 0x18, 0x22, 0x68, 0x60, 0x9f, 0x57, 0x42, 0xef, 0xc2, 0x51, 0xdc, 0x4f, 0xfd, 0x2e, 0xe9,
  0x2d, 0x06, 0x17, 0xd5, 0xc1, 0x53, 0xbd, 0xa6, 0xaa, 0x93, 0x2a, 0x6b, 0x8b, 0x90, 0xa8, 0x39,
  0xae, 0xa7, 0x27, 0x5c, 0x96, 0x57, 0xe3, 0x13, 0x11, 0x63, 0xaa, 0xdd, 0x51, 0x00, 0xe2, 0x6c,
  0xfd, 0x00, 0x20, 0xda, 0x6f, 0x70, 0x05, 0x9b, 0xf6, 0x51, 0x19, 0x9d, 0x73, 0x7f, 0x15, 0x4e,
  0x32, 0xfe, 0x99, 0xae, 0x94, 0x86, 0xd2, 0x3c, 0x33, 0xbb, 0x71, 0xb8, 0xf6, 0x1f, 0x5c, 0x85,
  0x2b, 0x43, 0xda, 0x50, 0xc7, 0x1b, 0x4f, 0x0e, 0x9c, 0x48, 0xc7, 0x01, 0x21, 0x8d, 0x78, 0x21,
  0xd2, 0x65, 0x8a, 0x63, 0x09, 0xdf, 0x9f, 0xf3, 0x0c, 0x99, 0x57, 0x79, 0xae, 0xe1, 0xd5, 0xe6,
  0x8b, 0x9c, 0x72, 0xfe, 0x77, 0x81, 0xa2, 0x00, 0x08, 0xe9, 0x32, 0x0b, 0x88, 0x4f, 0xfb, 0xad,
  0x0c, 0x4e, 0xee, 0x9b, 0x77, 0xec, 0x88, 0x47, 0x9c, 0xd3, 0x36, 0x2f, 0xe2, 0x65, 0xb1, 0xf3,
  0xee, 0xf5, 0xd2, 0x0b, 0x84, 0x6a, 0xeb, 0x55, 0x76, 0x6a, 0x7d, 0x73, 0xcc, 0x1b, 0x99, 0x8b,
  0x9f, 0xe8, 0x64, 0x45, 0x8e, 0x34, 0x00, 0xb8, 0xc9, 0x4e, 0xef, 0x7c, 0xaf, 0x9a, 0xa6, 0x1c,
  0x17, 0x79, 0x2e, 0xd7, 0xf0, 0xb6, 0xbb, 0x2a, 0x13, 0xfa, 0xa8, 0xec, 0x14, 0xc6, 0xbf, 0x9f,
  0x0a, 0x9c, 0x6b, 0x11, 0x6d, 0xe1, 0x40, 0x59, 0x06, 0x3a, 0x29, 0xfd, 0x7f, 0xb8, 0xe9, 0x39,
  0x72, 0x7f, 0xab, 0xc6, 0x88, 0x4b, 0xf3, 0xd4, 0xab, 0x95, 0xa0, 0xd1, 0x67, 0x19, 0xb2, 0xf3,
  0x79, 0x7b, 0x1f, 0xe6, 0xd0, 0xdd, 0x9e, 0x2e, 0x9c, 0xa0, 0xc0, 0x48, 0xa9, 0x2d, 0x91, 0x6d,
  0x3d, 0xbf, 0x9c, 0x5b, 0xe7, 0xbb, 0x59, 0xc3, 0xce, 0xff, 0xdd, 0x29, 0x15, 0x04, 0x27, 0xea,
  0xea, 0xa5, 0x25, 0xae, 0x57, 0x2b, 0xfc, 0x6a, 0x87, 0xa3, 0x45, 0x67, 0x80, 0x1a, 0xc9, 0xc0,
  0x3f, 0x41, 0xf7, 0x17, 0xc7, 0xb5, 0x76, 0x09, 0x28, 0x25, 0x95, 0x32, 0x4f, 0x29, 0x78, 0xb6,
  0xc0, 0x09, 0xa0, 0x84, 0xc9, 0x12, 0x8f, 0x37, 0x75, 0xa7, 0xcb, 0x4a, 0x21, 0x30, 0x5e, 0xfc,
  0x0c, 0xb8, 0x9e, 0xd5, 0x05, 0x55, 0xda, 0x84, 0x1e, 0x9b, 0x0c, 0xd5, 0xd8, 0x59, 0xc8, 0xae,
  0xc0, 0x69, 0xa3, 0xdb, 0x38, 0x86, 0xcf, 0xb0, 0xd0, 0xcb, 0x13, 0x02, 0x94, 0xfa, 0xb9, 0x38,
  0x39, 0x44, 0xbd, 0x82, 0xce, 0xd4, 0x89, 0xf0, 0xbd, 0xb2, 0x70, 0x08, 0x9f, 0x12, 0xb5, 0x13,
  0x3d, 0xf7, 0x39, 0x4e, 0x2d, 0xcd, 0x5a, 0x83, 0x70, 0x9d, 0x9f, 0xaa, 0x1c, 0x0a, 0x3c, 0xc8,
  0x88, 0xad, 0x91, 0xae, 0xa8, 0x23, 0xca, 0xbd, 0x61, 0x10, 0xa3, 0xc7, 0x80, 0x55, 0x90, 0xc8,
  0x0d, 0x45, 0x4f, 0x5f, 0x1a, 0x84, 0x18, 0xb1, 0x0f, 0x21, 0x1c, 0x22, 0xa1, 0x88, 0x0d, 0xe2,
  0x4a, 0x8d, 0x79, 0x08, 0x18, 0xc2, 0x05, 0x85, 0x6b, 0x7b, 0x47, 0x36, 0x76, 0x2f, 0x43, 0xd2,
  0xbb, 0xf4, 0x23, 0xba, 0x94, 0x7d, 0xe3, 0x36, 0x1d, 0xcb, 0x3d, 0xa9, 0xe2, 0xb4, 0x66, 0x8b,
  0x48, 0x0d, 0x79, 0xe2, 0x62, 0xdb, 0xe9, 0x96, 0xbd, 0x6e, 0x93, 0xa6, 0x48, 0xd1, 0x6d, 0x2f,
  0x38, 0x86, 0x82, 0x43, 0xda, 0xea, 0x9f, 0x42, 0x49, 0x13, 0xb4, 0x4c, 0x24, 0xe1, 0x11, 0x0c,
  0x46, 0x1e, 0x93, 0xb4, 0xc8, 0xdc, 0x5b, 0xfd, 0xf4, 0x1a, 0x12, 0xcd, 0x07, 0xf0, 0x50, 0xb1,
  0xe3, 0x52, 0x00, 0xba, 0x93, 0xb4, 0xc7, 0xa6, 0x05, 0x54, 0xc6, 0xbc, 0x89, 0x48, 0x69, 0x2f,
  0xdb, 0x2d, 0x6e, 0x47, 0x77, 0x8e, 0x9e, 0x5d, 0x15, 0xd5, 0x8f, 0xb0, 0xce, 0x88, 0xa6, 0xbb,
  0x86, 0xec, 0x65, 0x56, 0x44, 0xea, 0x74, 0x50, 0x1b, 0x7b, 0x3b, 0xbe, 0x70, 0xec, 0xbb, 0x86,
  0x4b, 0xd1, 0x47, 0x9c, 0x77, 0xe5, 0xee, 0xbf, 0x57, 0x30, 0x65, 0xb1, 0xf9, 0x3e, 0xb9, 0x27,
  0xdb, 0x45, 0xfa, 0x34, 0x52, 0x86, 0x2f, 0x5b, 0x31, 0xec, 0x91, 0x92, 0xfc, 0x8a, 0x14, 0x8d,
  0xb7, 0xf3, 0xa8, 0xe7, 0xd4, 0x09, 0xbf, 0x05, 0xf9, 0x36, 0xd7, 0x8d, 0xe5, 0x5f, 0x06, 0x71,
  0xf0, 0x97, 0x8c, 0x0f, 0xab, 0x0f, 0x1a, 0x80, 0x01, 0x45, 0xa5, 0x07, 0x25, 0x85, 0xe2, 0x6d,
  0xb6, 0xe6, 0x99, 0x07, 0x5b, 0xa6, 0x65, 0x7d, 0xb3, 0xea, 0xb8, 0x3b, 0xd4, 0xe1, 0xc8, 0x45,
  0xd4, 0xf8, 0xc1, 0x89, 0x86, 0x50, 0xef, 0xc7, 0x05, 0x1a, 0x16, 0x8a, 0xc6, 0x5f, 0xcf, 0x0b,
  0xcd, 0x8c, 0x67, 0xe7, 0x40, 0x6b, 0x98, 0x27, 0xaa, 0x8f, 0x3d, 0x7f, 0x65, 0x67, 0xc9, 0xcf,
  0xa2, 0xa2, 0xb7, 0xb5, 0xd8, 0xde, 0x24, 0xb9, 0x74, 0xf6, 0xb4, 0xcf, 0x75, 0x52, 0x60, 0x33,
  0xe8, 0xda, 0x1d, 0xb0, 0x06, 0x18, 0xb1, 0x54, 0x67, 0x60, 0x47, 0x9e, 0xeb, 0xa2, 0x68, 0xe7,
  0x71, 0x5c, 0x82, 0xd6, 0xa8, 0xac, 0x7c, 0x93, 0x79, 0xcf, 0x2d, 0x7e, 0x16, 0xf1, 0x8f, 0x89,
  0xd6, 0x43, 0xbe, 0x61, 0x16, 0x35, 0xd8, 0x6a, 0xd0, 0xab, 0x6f, 0x82, 0x31, 0x0b, 0xd9, 0x0e,
  0xf8, 0x81, 0x8f, 0x38, 0xe2, 0xdd, 0x1f, 0x32, 0xa8, 0x1a, 0xe8, 0xab, 0x38, 0xd7, 0x56, 0x5f,
  0x99, 0x6c, 0x97, 0x30, 0x3f, 0xab, 0x5a, 0xc1, 0x21, 0x47, 0x34, 0x40, 0x52, 0x4c, 0xb3, 0xb0,
  0xb4, 0x74, 0xc6, 0x1d, 0x62, 0xba, 0xbf, 0xf8, 0x99, 0xfb, 0xbb, 0x1f, 0x66, 0x12, 0x65, 0x14,
  0xf0, 0x04, 0x44, 0x37, 0xdd, 0x79, 0x31, 0xab, 0x7b, 0x2f, 0xb1, 0x73, 0x2f, 0xf7, 0x7b, 0xfe,
  0xbb, 0x16, 0x95, 0xe1, 0x4d, 0x43, 0xc9, 0xf8, 0x8a, 0x74, 0x18, 0x53, 0xd0, 0x82, 0x13, 0xe5,
  0x74, 0x1b, 0xa5, 0x14, 0x72, 0xe9, 0xe1, 0x87, 0xd7, 0xd9, 0xda, 0x89, 0xa2, 0x3a, 0x12, 0xc3,
  0xfc, 0x7c, 0x0d, 0x60, 0x91, 0x09, 0xc5, 0xf8, 0xf5, 0xed, 0xba, 0x59, 0x32, 0x19, 0x0d, 0xbe,
  0xb7, 0xdd, 0xf8, 0x72, 0x6f, 0x25, 0x9e, 0x56, 0x31, 0xe9, 0xe8, 0xa6, 0x4f, 0xbd, 0x62, 0x69,
  0xcf, 0xa0, 0x59, 0x77, 0x14, 0xbe, 0x0a, 0xa7, 0xb5, 0x7c, 0xdd, 0xc1, 0xcb, 0xd8, 0x14, 0x70,
  0xb1, 0xf1, 0xb9, 0x7e, 0x95, 0x80, 0x37, 0xf9, 0x2d, 0xf2, 0xb0, 0x32, 0x71, 0xd9, 0xce, 0x29,
  0xef, 0xe1, 0x6a, 0x3d, 0x94, 0x1e, 0xc3, 0xb3, 0x2a, 0x85, 0xb1, 0xff, 0xa8, 0x16, 0xf9, 0xce,
  0x56, 0x23, 0x7b, 0xfc, 0x2c, 0x59, 0x5d, 0xab, 0x22, 0xd9, 0x65, 0x71, 0x4f, 0x3a, 0x7c, 0xb4,
  0x7b, 0x7d, 0xec, 0xd4, 0x13, 0xc4, 0xea, 0x9c, 0x55, 0x03, 0x10, 0xe2, 0xf2, 0x4f, 0x31, 0x6f,
  0x73, 0xb3, 0xbd, 0x79, 0xbd, 0x45, 0x3a, 0xa5, 0xad, 0xa9, 0x87, 0xcf, 0x86, 0x5f, 0xac, 0x25,
  0x23, 0x3c, 0x59, 0x3c, 0x56, 0xd9, 0x82, 0x2f, 0xc3, 0x41, 0x92, 0x26, 0xa7, 0xb6, 0x2b, 0xba,
  0x30, 0xa7, 0xf7, 0xc6, 0xa4, 0xe9, 0x17, 0xa0, 0xbc, 0xb4, 0x8e, 0x6a, 0x69, 0xca, 0xf6, 0xe5,
  0x0f, 0xd1, 0x52, 0xe1, 0xa5, 0x13, 0xc7, 0xcc, 0xf7, 0x1c, 0x3b, 0xb4, 0xdb, 0xa2, 0x16, 0x7c,
  0x34, 0x9f, 0x26, 0xe2, 0xd7, 0xac, 0xd1, 0xaf, 0xf6, 0xbd, 0x19, 0x2b, 0x28, 0xdc, 0x5b, 0xfc,
  0x24, 0xf6, 0x16, 0x3f, 0xc5, 0xd3, 0x37, 0xcb, 0x01, 0x2e, 0x9a, 0xa2, 0x96, 0x3c, 0x98, 0xc9,
  0x98, 0x8b, 0x87, 0x24, 0x62, 0x7b, 0xb4, 0x26, 0x53, 0x6c, 0xde, 0xf1, 0x69, 0xff, 0x11, 0x1e,
  0xa6, 0x18, 0x22, 0x68, 0x60, 0x9f, 0x57, 0x42, 0xef, 0xc2, 0x51, 0xdc, 0x4f, 0xfd, 0x2e, 0xe9,
  0x2d, 0x06, 0x17, 0xd5, 0xc1, 0x53, 0xbd, 0xa6, 0xaa, 0x93, 0x2a, 0x6b, 0x8b, 0x90, 0xa8, 0x39,
  0xae, 0xa7, 0x27, 0x5c, 0x96, 0x57, 0xe3, 0x13, 0x11, 0x63, 0xaa, 0xdd, 0x51, 0x00, 0xe2, 0x6c,
  0xfd, 0x00, 0x20, 0xda, 0x6f, 0x70, 0x05, 0x9b, 0xf6, 0x51, 0x19, 0x9d, 0x73, 0x7f, 0x15, 0x4e,
  0x32, 0xfe, 0x99, 0xae, 0x94, 0x86, 0xd2, 0x3c, 0x33, 0xbb, 0x71, 0xb8, 0xf6, 0x1f, 0x5c, 0x85,
  0x2b, 0x43, 0xda, 0x50, 0xc7, 0x1b, 0x4f, 0x0e, 0x9c, 0x48, 0xc7, 0x01, 0x21, 0x8d, 0x78, 0x21,
  0xd2, 0x65, 0x8a, 0x63, 0x09, 0xdf, 0x9f, 0xf3, 0x0c, 0x99, 0x57, 0x79, 0xae, 0xe1, 0xd5, 0xe6,
  0x8b, 0x9c, 0x72, 0xfe, 0x77, 0x81, 0xa2, 0x00, 0x08, 0xe9, 0x32, 0x0b, 0x88, 0x4f, 0xfb, 0xad,
  0x0c, 0x4e, 0xee, 0x9b, 0x77, 0xec, 0x88, 0x47, 0x9c, 0xd3, 0x36, 0x2f, 0xe2, 0x65, 0xb1, 0xf3,
  0xee, 0xf5, 0xd2, 0x0b, 0x84, 0x6a, 0xeb, 0x55, 0x76, 0x6a, 0x7d, 0x73, 0xcc, 0x1b, 0x99, 0x8b,
  0x9f, 0xe8, 0x64, 0x45, 0x8e, 0x34, 0x00, 0xb8, 0xc9, 0x4e, 0xef, 0x7c, 0xaf, 0x9a, 0xa6, 0x1c,
  0x17, 0x79, 0x2e, 0xd7, 0xf0, 0xb6, 0xbb, 0x2a, 0x13, 0xfa, 0xa8, 0xec, 0x14, 0xc6, 0xbf, 0x9f,
  0x0a, 0x9c, 0x6b, 0x11, 0x6d, 0xe1, 0x40, 0x59, 0x06, 0x3a, 0x29, 0xfd, 0x7f, 0xb8, 0xe9, 0x39,
  0x72, 0x7f, 0xab, 0xc6, 0x88, 0x4b, 0xf3, 0xd4, 0xab, 0x95, 0xa0, 0xd1, 0x67, 0x19, 0xb2, 0xf3,
  0x79, 0x7b, 0x1f, 0xe6, 0xd0, 0xdd, 0x9e, 0x2e, 0x9c, 0xa0, 0xc0, 0x48, 0xa9, 0x2d, 0x91, 0x6d,
  0x3d, 0xbf, 0x9c, 0x5b, 0xe7, 0xbb, 0x59, 0xc3, 0xce, 0xff, 0xdd, 0x29, 0x15, 0x04, 0x27, 0xea,
  0xea, 0xa5, 0x25, 0xae, 0x57, 0x2b, 0xfc, 0x6a, 0x87, 0xa3, 0x45, 0x67, 0x80, 0x1a, 0xc9, 0xc0,
  0x3f, 0x41, 0xf7, 0x17, 0xc7, 0xb5, 0x76, 0x09, 0x28, 0x25, 0x95, 0x32, 0x4f, 0x29, 0x78, 0xb6,
  0xc0, 0x09, 0xa0, 0x84, 0xc9, 0x12, 0x8f, 0x37, 0x75, 0xa7, 0xcb, 0x4a, 0x21, 0x30, 0x5e, 0xfc,
  0x0c, 0xb8, 0x9e, 0xd5, 0x05, 0x55, 0xda, 0x84, 0x1e, 0x9b, 0x0c, 0xd5, 0xd8, 0x59, 0xc8, 0xae,
  0xc0, 0x69, 0xa3, 0xdb, 0x38, 0x86, 0xcf, 0xb0, 0xd0, 0xcb, 0x13, 0x02, 0x94, 0xfa, 0xb9, 0x38,
  0x39, 0x44, 0xbd, 0x82, 0xce, 0xd4, 0x89, 0xf0, 0xbd, 0xb2, 0x70, 0x08, 0x9f, 0x12, 0xb5, 0x13,
  0x3d, 0xf7, 0x39, 0x4e, 0x2d, 0xcd, 0x5a, 0x83, 0x70, 0x9d, 0x9f, 0xaa, 0x1c, 0x0a, 0x3c, 0xc8,
  0x88, 0xad, 0x91, 0xae, 0xa8, 0x23, 0xca, 0xbd, 0x61, 0x10, 0xa3, 0xc7, 0x80, 0x55, 0x90, 0xc8,
  0x0d, 0x45, 0x4f, 0x5f, 0x1a, 0x84, 0x18, 0xb1, 0x0f, 0x21, 0x1c, 0x22, 0xa1, 0x88, 0x0d, 0xe2,
  0x4a, 0x8d, 0x79, 0x08, 0x18, 0xc2, 0x05, 0x85, 0x6b, 0x7b, 0x47, 0x36, 0x76, 0x2f, 0x43, 0xd2,
  0xbb, 0xf4, 0x23, 0xba, 0x94, 0x7d, 0xe3, 0x36, 0x1d, 0xcb, 0x3d, 0xa9, 0xe2, 0xb4, 0x66, 0x8b,
  0x48, 0x0d, 0x79, 0xe2, 0x62, 0xdb, 0xe9, 0x96, 0xbd, 0x6e, 0x93, 0xa6, 0x48, 0xd1, 0x6d, 0x2f,
  0x38, 0x86, 0x82, 0x43, 0xda, 0xea, 0x9f, 0x42, 0x49, 0x13, 0xb4, 0x4c, 0x24, 0xe1, 0x11, 0x0c,
  0x46, 0x1e, 0x93, 0xb4, 0xc8, 0xdc, 0x5b, 0xfd, 0xf4, 0x1a, 0x12, 0xcd, 0x07, 0xf0, 0x50, 0xb1,
  0xe3, 0x52, 0x00, 0xba, 0x93, 0xb4, 0xc7, 0xa6, 0x05, 0x54, 0xc6, 0xbc, 0x89, 0x48, 0x69, 0x2f,
  0xdb, 0x2d, 0x6e, 0x47, 0x77, 0x8e, 0x9e, 0x5d, 0x15, 0xd5, 0x8f, 0xb0, 0xce, 0x88, 0xa6, 0xbb,
  0x86, 0xec, 0x65, 0x56, 0x44, 0xea, 0x74, 0x50, 0x1b, 0x7b, 0x3b, 0xbe, 0x70, 0xec, 0xbb, 0x86,
  0x4b, 0xd1, 0x47, 0x9c, 0x77, 0xe5, 0xee, 0xbf, 0x57, 0x30, 0x65, 0xb1, 0xf9, 0x3e, 0xb9, 0x27,
};

static int datasize = sizeof(data);

static WINDOW *hexwin;
static WINDOW *statuswin;

enum {
  DEFAULT_COLOR_PAIR = 1,
  STATUS_COLOR_PAIR,
  CURSOR_COLOR_PAIR,
};

#define status(...) do { werase(statuswin); wprintw(statuswin, __VA_ARGS__); wrefresh(statuswin); refresh(); } while(0);

static void status_init()
{
  statuswin = newwin(1, COLS, LINES-1, 0);
  if (has_colors()) {
    init_pair(STATUS_COLOR_PAIR, COLOR_BLACK, COLOR_WHITE);
    wcolor_set(statuswin, STATUS_COLOR_PAIR, NULL);
  }
  wrefresh(statuswin);
}

static int hexwin_offset = 0;
static int hexwin_cursor = 0;

#define BYTES_PER_LINE		16
#define BYTES_GROUP_BY		4
#define HEXWIN_LINES		(LINES-5)
#define PGSKIP			16
#define BYTES_PER_SCREEN	(BYTES_PER_LINE*HEXWIN_LINES)

static void hexwin_init(void)
{
  hexwin = newwin(HEXWIN_LINES+2, BYTES_PER_LINE*3 + BYTES_PER_LINE/BYTES_GROUP_BY, 2, 0);
  scrollok(hexwin, TRUE);
  idlok(hexwin, TRUE);
  box(hexwin, 0, 0);
  wrefresh(hexwin);

  if (has_colors())
    init_pair(CURSOR_COLOR_PAIR, COLOR_BLACK, COLOR_YELLOW);
}

static void hexwin_redraw(void)
{
  werase(hexwin);
  box(hexwin, 0, 0);
  int y = 0;
  for(int i=hexwin_offset; i<datasize/BYTES_PER_LINE; i++, y++) {
    if (y >= HEXWIN_LINES)
      break;
    for (int j=0; j<BYTES_PER_LINE; j++) {
      mvwprintw(hexwin, y+1, j*3+1 + j/BYTES_GROUP_BY, "%02x", (unsigned char) data[i*BYTES_PER_LINE + j]);
      if (i*BYTES_PER_LINE + j == hexwin_cursor)
	mvwchgat(hexwin, y+1, j*3+1 + j/BYTES_GROUP_BY, 2, 0, CURSOR_COLOR_PAIR, NULL);
    }
  }

  wrefresh(hexwin);
}

static void hexwin_scroll_to_cursor(void) {
  while (hexwin_offset*BYTES_PER_LINE > hexwin_cursor)
    hexwin_offset--;
  while (hexwin_cursor - hexwin_offset*BYTES_PER_LINE >= BYTES_PER_SCREEN)
    hexwin_offset++;
}

static void hexwin_cursor_to_scroll(void) {
  while (hexwin_offset*BYTES_PER_LINE > hexwin_cursor)
    hexwin_cursor += BYTES_PER_LINE;
  while (hexwin_cursor - hexwin_offset*BYTES_PER_LINE >= BYTES_PER_SCREEN)
    hexwin_cursor -= BYTES_PER_LINE;
}

int main(int argc, char **argv)
{
  initscr();
  start_color();
  curs_set(0);
  cbreak();
  noecho();
  
  keypad(stdscr, TRUE);
  refresh();

  status_init();
  hexwin_init();
  hexwin_redraw();

  while (1) {
    int ch = getch();
//    status("Hit key: %x", ch);
    switch(ch) {
      case 'q':
	endwin();
	return 0;
      case KEY_PPAGE:
	hexwin_offset -= PGSKIP;
	if (hexwin_offset < 0)
	  hexwin_offset = 0;
	hexwin_cursor_to_scroll();
	break;
      case KEY_NPAGE:
	hexwin_offset += PGSKIP;
	if (hexwin_offset >= datasize/BYTES_PER_LINE)
	  hexwin_offset = datasize/BYTES_PER_LINE - 1;
	hexwin_cursor_to_scroll();
	break;
      case KEY_LEFT:
	hexwin_cursor--;
	if (hexwin_cursor < 0)
	  hexwin_cursor = 0;
	hexwin_scroll_to_cursor();
	break;
      case KEY_RIGHT:
	hexwin_cursor++;
	if (hexwin_cursor >= datasize)
	  hexwin_cursor = datasize-1;
	hexwin_scroll_to_cursor();
	break;
      case KEY_UP:
	hexwin_cursor -= BYTES_PER_LINE;
	if (hexwin_cursor < 0)
	  hexwin_cursor = 0;
	hexwin_scroll_to_cursor();
	break;
      case KEY_DOWN:
	hexwin_cursor += BYTES_PER_LINE;
	if (hexwin_cursor >= datasize)
	  hexwin_cursor = datasize - 1;
	hexwin_scroll_to_cursor();
	break;
    }
    hexwin_redraw();
//    refresh();
  }
}
